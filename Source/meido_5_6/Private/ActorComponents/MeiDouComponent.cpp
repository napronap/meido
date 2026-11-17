// Fill out your copyright notice in the Description page of Project Settings.

#include "ActorComponents/MeiDouComponent.h"

#include "ActorComponents/AttackComponent.h"
#include "ActorComponents/LockOnComponent.h"
#include "Characters/MaidCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Interfaces/MeiDouSpawnConfigurable.h"
#include "Kismet/GameplayStatics.h"
#include "MeiDouPoseDataAsset.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"

UMeiDouComponent::UMeiDouComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

const TArray<EMeiDouInput>& UMeiDouComponent::GetInputBuffer() const
{
	return ComboInputBuffer;
}

int32 UMeiDouComponent::GetInputCount(const EMeiDouInput Input) const
{
	int32 Count = 0;
	for (const EMeiDouInput Buffered : ComboInputBuffer)
	{
		if (Buffered == Input)
		{
			++Count;
		}
	}
	return Count;
}

bool UMeiDouComponent::ShouldMirrorPose(const EMeiDouInput Input) const
{
	// Doc §9.6: CountP = how many P already in buffer; if CountP % 2 == 1 → mirror
	// (2nd, 4th… occurrence of that pose in the string).
	if (Input == EMeiDouInput::None)
	{
		return false;
	}
	const int32 CountAlreadyInBuffer = GetInputCount(Input);
	return (CountAlreadyInBuffer % 2) == 1;
}

const UMeiDouPoseDataAsset* UMeiDouComponent::FindPoseData(const EMeiDouInput Input) const
{
	if (const TObjectPtr<UMeiDouPoseDataAsset>* Found = PoseDataMap.Find(Input))
	{
		return Found->Get();
	}
	if (const AMaidCharacter* Maid = GetMaidOwner())
	{
		return Maid->MeiDouPoseDataMap.FindRef(Input);
	}
	return nullptr;
}

UAnimMontage* UMeiDouComponent::GetFailMontage() const
{
	if (FailMontage)
	{
		return FailMontage.Get();
	}
	if (const AMaidCharacter* Maid = GetMaidOwner())
	{
		return Maid->MeiDouFailMontage;
	}
	return nullptr;
}

void UMeiDouComponent::BeginPlay()
{
	Super::BeginPlay();
	ComboInputBuffer.Reserve(MaxInputs);
	QueuedPoseInputs.Reserve(MaxInputs);
	ClearActiveComboDefinition();
	ClearPendingResult();
	ClearPendingFail();
	SpawnedResultActor.Reset();
	bDestroySpawnedResultActorOnActionEnd = false;
	ClearComboInputTimeout();
	SetMeiDouState(EMeiDouState::EMDS_Idle);

	if (bPreloadResultsOnBeginPlay)
	{
		PreloadResults();
	}
}

void UMeiDouComponent::PreloadResults()
{
	if (ComboData)
	{
		ComboData->PreloadSpawnClasses();
	}

	bResultsPreloaded = true;
	DebugMessage(TEXT("MeiDou: result spawn classes preloaded"), FColor::Cyan, 1.5f);
}

bool UMeiDouComponent::RegisterInput(const EMeiDouInput Input)
{
	if (Input == EMeiDouInput::None)
	{
		return false;
	}

	if (QueuedPoseInputs.Num() >= MaxInputs)
	{
		return false;
	}

	// Committing a pose cancels the inter-pose wait timer (new input is progress).
	ClearComboInputTimeout();
	QueuedPoseInputs.Add(Input);
	AdvanceFromIdle();
	return true;
}

void UMeiDouComponent::OnMeiDouActionWindowBegin()
{
	// Policy 1: never run inter-pose timeout during pose/result window.
	ClearComboInputTimeout();

	OnMeiDouControlLockChanged.Broadcast(true);

	if (bHasActiveComboDefinition)
	{
		SetMeiDouState(EMeiDouState::EMDS_Result);
		return;
	}

	SetMeiDouState(EMeiDouState::EMDS_Posing);
}

void UMeiDouComponent::OnMeiDouActionWindowEnd()
{
	OnMeiDouControlLockChanged.Broadcast(false);

	if (AActor* OwnerActor = GetOwner())
	{
		if (UAttackComponent* AttackComponent = OwnerActor->FindComponentByClass<UAttackComponent>())
		{
			AttackComponent->CloseHitWindow();
		}
	}

	CleanupSpawnedResultActor();

	if (MeiDouState == EMeiDouState::EMDS_Result)
	{
		ClearActiveComboDefinition();
	}

	// Policy 2: return to Idle after pose/result ends; Advance arms timeout only if still waiting.
	SetMeiDouState(EMeiDouState::EMDS_Idle);
	AdvanceFromIdle();
}

void UMeiDouComponent::OnRequestedAnimationFailed()
{
	OnMeiDouControlLockChanged.Broadcast(false);

	if (AActor* OwnerActor = GetOwner())
	{
		if (UAttackComponent* AttackComponent = OwnerActor->FindComponentByClass<UAttackComponent>())
		{
			AttackComponent->CloseHitWindow();
		}
	}

	CleanupSpawnedResultActor();

	if (MeiDouState == EMeiDouState::EMDS_Result)
	{
		ClearActiveComboDefinition();
	}

	SetMeiDouState(EMeiDouState::EMDS_Idle);
	AdvanceFromIdle();
}

void UMeiDouComponent::ResetCombo()
{
	ComboInputBuffer.Reset();
	ClearComboInputTimeout();
}

void UMeiDouComponent::ResetForFlowRestart()
{
	ClearComboInputTimeout();
	QueuedPoseInputs.Reset();
	ClearActiveComboDefinition();
	ClearPendingResult();
	ClearPendingFail();
	SpawnedResultActor.Reset();
	bDestroySpawnedResultActorOnActionEnd = false;
	const TArray<TWeakObjectPtr<AActor>> ResultsToReset = RunSpawnedResultActors;
	RunSpawnedResultActors.Reset();
	for (const TWeakObjectPtr<AActor>& ResultEntry : ResultsToReset)
	{
		if (AActor* RunResultActor = ResultEntry.Get())
		{
			RunResultActor->Destroy();
		}
	}
	ResetComboAndNotify(EMeiDouBufferChangeReason::ClearedOther);
	SetMeiDouState(EMeiDouState::EMDS_Idle);
	OnMeiDouControlLockChanged.Broadcast(false);
}

void UMeiDouComponent::ResetComboAndNotify(const EMeiDouBufferChangeReason Reason)
{
	ResetCombo();
	NotifyBufferChanged(Reason);
}

void UMeiDouComponent::NotifyBufferChanged(const EMeiDouBufferChangeReason Reason)
{
	OnMeiDouBufferChanged.Broadcast(ComboInputBuffer, Reason);
}

void UMeiDouComponent::HandleAnimEvent(const EMeiDouAnimEvent EventKey)
{
	AActor* OwnerActor = GetOwner();
	UAttackComponent* AttackComponent = OwnerActor
		? OwnerActor->FindComponentByClass<UAttackComponent>()
		: nullptr;

	switch (EventKey)
	{
	case EMeiDouAnimEvent::EMDAE_Spawn:
		if (!bHasActiveComboDefinition)
		{
			DebugMessage(TEXT("MeiDou Spawn notify ignored: no active combo"), FColor::Red);
			return;
		}

		if (ActiveComboDefinition.ResultType == EMeiDouResultType::EMDRT_Spawn)
		{
			ExecuteSpawnResult(ActiveComboDefinition.SpawnConfig);
		}
		else
		{
			DebugMessage(TEXT("MeiDou Spawn notify ignored: result is not Spawn"), FColor::Yellow);
		}
		break;

	case EMeiDouAnimEvent::EMDAE_ControlEnable:
	case EMeiDouAnimEvent::EMDAE_ControlDisable:
		break;

	case EMeiDouAnimEvent::EMDAE_TraceStart:
	{
		if (!bHasActiveComboDefinition || ActiveComboDefinition.ResultType != EMeiDouResultType::EMDRT_Damage)
		{
			break;
		}

		if (!AttackComponent)
		{
			DebugMessage(TEXT("MeiDou TraceStart ignored: missing AttackComponent"), FColor::Red);
			break;
		}

		AttackComponent->StartAttack(EHitStopType::Heavy);

		TArray<FName> DamageSockets = ActiveComboDefinition.DamageConfig.SocketNames;
		if (DamageSockets.Num() <= 0 && ActiveComboDefinition.DamageConfig.SocketName != NAME_None)
		{
			DamageSockets.Add(ActiveComboDefinition.DamageConfig.SocketName);
		}

		AttackComponent->OpenHitWindowSockets(DamageSockets);
		break;
	}

	case EMeiDouAnimEvent::EMDAE_TraceEnd:
	{
		if (!AttackComponent)
		{
			break;
		}

		if (!bHasActiveComboDefinition || ActiveComboDefinition.ResultType != EMeiDouResultType::EMDRT_Damage)
		{
			break;
		}

		AttackComponent->CloseHitWindow();
		break;
	}

	default:
		break;
	}
}

const FMeiDouComboDefinition* UMeiDouComponent::GetActiveComboDefinition() const
{
	return bHasActiveComboDefinition ? &ActiveComboDefinition : nullptr;
}

void UMeiDouComponent::SetMeiDouState(const EMeiDouState NewState)
{
	if (MeiDouState == NewState)
	{
		return;
	}

	MeiDouState = NewState;
	OnMeiDouStateChanged.Broadcast(MeiDouState);
}

void UMeiDouComponent::ClearActiveComboDefinition()
{
	ActiveComboDefinition = FMeiDouComboDefinition();
	bHasActiveComboDefinition = false;
}

void UMeiDouComponent::ClearPendingResult()
{
	PendingResultDefinition = FMeiDouComboDefinition();
	PendingResultPayload = FMeiDouResolvedCombo();
	bHasPendingResult = false;
}

void UMeiDouComponent::ClearPendingFail()
{
	PendingFailPayload = FMeiDouResolvedCombo();
	bHasPendingFail = false;
}

void UMeiDouComponent::CleanupSpawnedResultActor()
{
	if (!SpawnedResultActor.IsValid())
	{
		bDestroySpawnedResultActorOnActionEnd = false;
		return;
	}

	AActor* SpawnedActor = SpawnedResultActor.Get();
	if (SpawnedActor->GetClass()->ImplementsInterface(UMeiDouSpawnConfigurable::StaticClass()))
	{
		IMeiDouSpawnConfigurable::Execute_OnMeiDouActionEnded(SpawnedActor);
	}

	if (bDestroySpawnedResultActorOnActionEnd && IsValid(SpawnedActor))
	{
		SpawnedActor->Destroy();
	}

	SpawnedResultActor.Reset();
	bDestroySpawnedResultActorOnActionEnd = false;
}

void UMeiDouComponent::ArmInterPoseTimeoutIfWaiting()
{
	if (ComboInputTimeoutSeconds <= 0.f)
	{
		return;
	}

	if (bTimeoutOnlyWhileIdleWaiting)
	{
		// Policy 1+2: only while Idle, after a pose finished, waiting for the next input.
		if (MeiDouState != EMeiDouState::EMDS_Idle)
		{
			return;
		}
		if (QueuedPoseInputs.Num() > 0 || bHasPendingResult || bHasPendingFail)
		{
			return;
		}
		if (ComboInputBuffer.Num() <= 0 || ComboInputBuffer.Num() >= MaxInputs)
		{
			return;
		}
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	World->GetTimerManager().SetTimer(
		ComboInputTimeoutHandle,
		this,
		&UMeiDouComponent::OnComboInputTimeout,
		ComboInputTimeoutSeconds,
		false
	);

	DebugMessage(
		FString::Printf(TEXT("MeiDou: inter-pose timeout armed (%.2fs)"), ComboInputTimeoutSeconds),
		FColor::Cyan,
		1.0f
	);
}

void UMeiDouComponent::ClearComboInputTimeout()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ComboInputTimeoutHandle);
	}
}

void UMeiDouComponent::OnComboInputTimeout()
{
	// Guard: never apply timeout mid-pose/result even if a stale timer fired.
	if (bTimeoutOnlyWhileIdleWaiting && MeiDouState != EMeiDouState::EMDS_Idle)
	{
		return;
	}

	if (ComboInputBuffer.Num() <= 0 && QueuedPoseInputs.Num() <= 0)
	{
		return;
	}

	DebugMessage(TEXT("MeiDou combo timeout: buffer + queue reset"), FColor::Yellow, 1.5f);
	QueuedPoseInputs.Reset();
	ResetComboAndNotify(EMeiDouBufferChangeReason::ClearedTimeout);
}

void UMeiDouComponent::AdvanceFromIdle()
{
	// Only advance the pipeline while Idle. Posing/Result wait for notifies.
	if (MeiDouState != EMeiDouState::EMDS_Idle)
	{
		return;
	}

	// 1) Next pose from input queue
	if (QueuedPoseInputs.Num() > 0)
	{
		ClearComboInputTimeout();

		const EMeiDouInput NextInput = QueuedPoseInputs[0];
		QueuedPoseInputs.RemoveAt(0);

		// Mirror before append (ShouldMirrorPose reads buffer as-is).
		const bool bMirror = ShouldMirrorPose(NextInput);
		AppendToComboBuffer(NextInput); // notifies PoseAdded (+ clear reason if resolved)

		FMeiDouPoseAnimationRequest Request;
		Request.Input = NextInput;
		Request.bUseMirroredMontage = bMirror;

		// Entering pose: no inter-pose timer (policy 1).
		ClearComboInputTimeout();
		SetMeiDouState(EMeiDouState::EMDS_Posing);
		PlayPoseSfx(NextInput, bMirror);
		OnPoseAnimationRequested.Broadcast(Request);
		PresentPose(Request);
		return;
	}

	// 2) Pending success → Result presentation
	if (bHasPendingResult)
	{
		ClearComboInputTimeout();

		ActiveComboDefinition = PendingResultDefinition;
		bHasActiveComboDefinition = true;
		const FMeiDouResolvedCombo ResolvedResult = PendingResultPayload;
		ClearPendingResult();

		SetMeiDouState(EMeiDouState::EMDS_Result);
		PlayMeiDouSfx(SuccessSfx.Get());
		OnComboResolved.Broadcast(ResolvedResult);
		PresentResult(ResolvedResult);
		DebugMessage(
			FString::Printf(TEXT("MeiDou SUCCESS: %s"), *ResolvedResult.ComboId.ToString()),
			FColor::Green
		);
		return;
	}

	// 3) Pending fail → berrinche
	if (bHasPendingFail)
	{
		ClearComboInputTimeout();

		const FMeiDouResolvedCombo FailedResult = PendingFailPayload;
		ClearPendingFail();

		PlayMeiDouSfx(FailSfx.Get());
		OnComboFailed.Broadcast(FailedResult);
		PresentFail(FailedResult);
		DebugMessage(TEXT("MeiDou FAIL"), FColor::Red);
		return;
	}

	// 4) Partial buffer, nothing else to do → arm timeout from *this* idle moment
	//    (typically right after pose ActionWindowEnd). Policy 2.
	ArmInterPoseTimeoutIfWaiting();
}

bool UMeiDouComponent::AppendToComboBuffer(const EMeiDouInput Input)
{
	if (ComboInputBuffer.Num() >= MaxInputs)
	{
		return false;
	}

	ComboInputBuffer.Add(Input);
	NotifyBufferChanged(EMeiDouBufferChangeReason::PoseAdded);

	if (ComboInputBuffer.Num() == MaxInputs)
	{
		return TryResolveBuffer();
	}

	return false;
}

bool UMeiDouComponent::TryResolveBuffer()
{
	if (!ComboData || ComboInputBuffer.Num() != MaxInputs)
	{
		return false;
	}

	if (const FMeiDouComboDefinition* Def = ComboData->FindDefinition(ComboInputBuffer))
	{
		QueueSuccess(*Def);
		PendingResultPayload.ComboId = Def->ComboId;
		PendingResultPayload.Inputs = ComboInputBuffer;
		ResetComboAndNotify(EMeiDouBufferChangeReason::ClearedSuccess);
		return true;
	}

	QueueFail();
	PendingFailPayload.ComboId = NAME_None;
	PendingFailPayload.Inputs = ComboInputBuffer;
	ResetComboAndNotify(EMeiDouBufferChangeReason::ClearedFail);
	return false;
}

void UMeiDouComponent::QueueSuccess(const FMeiDouComboDefinition& Definition)
{
	PendingResultDefinition = Definition;
	bHasPendingResult = true;
}

void UMeiDouComponent::QueueFail()
{
	bHasPendingFail = true;
}

void UMeiDouComponent::PresentPose(const FMeiDouPoseAnimationRequest& Request)
{
	if (AMaidCharacter* Maid = GetMaidOwner())
	{
		Maid->PlayMeiDouPose(Request);
	}
}

void UMeiDouComponent::PresentResult(const FMeiDouResolvedCombo& Result)
{
	if (AMaidCharacter* Maid = GetMaidOwner())
	{
		Maid->PlayMeiDouResult(Result);
	}
}

void UMeiDouComponent::PresentFail(const FMeiDouResolvedCombo& Result)
{
	if (AMaidCharacter* Maid = GetMaidOwner())
	{
		Maid->PlayMeiDouFail(Result);
	}
}

void UMeiDouComponent::PlayMeiDouSfx(USoundBase* Sound) const
{
	if (!Sound)
	{
		return;
	}

	AActor* OwnerActor = GetOwner();
	UWorld* World = GetWorld();
	if (!OwnerActor || !World)
	{
		return;
	}

	UGameplayStatics::PlaySoundAtLocation(World, Sound, OwnerActor->GetActorLocation());
}

void UMeiDouComponent::PlayPoseSfx(const EMeiDouInput Input, const bool bMirrored) const
{
	const UMeiDouPoseDataAsset* Pose = FindPoseData(Input);
	if (!Pose)
	{
		return;
	}

	USoundBase* Sound = nullptr;
	if (bMirrored && Pose->MirroredPoseSfx)
	{
		Sound = Pose->MirroredPoseSfx.Get();
	}
	else
	{
		Sound = Pose->PoseSfx.Get();
	}

	PlayMeiDouSfx(Sound);
}

AMaidCharacter* UMeiDouComponent::GetMaidOwner() const
{
	return Cast<AMaidCharacter>(GetOwner());
}

void UMeiDouComponent::DebugMessage(const FString& Message, const FColor& Color, const float Time) const
{
	if (!bDrawDebugMeiDou || !GEngine)
	{
		return;
	}
	GEngine->AddOnScreenDebugMessage(-1, Time, Color, Message);
}

void UMeiDouComponent::ExecuteSpawnResult(const FMeiDouSpawnConfig& SpawnConfig)
{
	UWorld* ResultWorld = GetWorld();
	AActor* OwnerActor = GetOwner();
	UClass* SpawnClass = SpawnConfig.ResolveSpawnClass();

	if (!ResultWorld || !OwnerActor || !SpawnClass)
	{
		DebugMessage(TEXT("MeiDou Spawn ignored: no owner/world/class (set Soft or hard SpawnedActorClass)"), FColor::Red);
		return;
	}

	AActor* TargetActor = nullptr;
	const FVector SpawnLocation = GetSpawnLocationForConfig(SpawnConfig, TargetActor);
	const FVector EndLocation = TargetActor
		? TargetActor->GetActorLocation()
		: (OwnerActor->GetActorLocation() + OwnerActor->GetActorForwardVector() * SpawnConfig.ForwardDistanceIfNoTarget);
	const FRotator SpawnRotation = OwnerActor->GetActorRotation();
	const FTransform SpawnTransform(SpawnRotation, SpawnLocation);

	APawn* InstigatorPawn = Cast<APawn>(OwnerActor);

	AActor* SpawnedActor = ResultWorld->SpawnActorDeferred<AActor>(
		SpawnClass,
		SpawnTransform,
		OwnerActor,
		InstigatorPawn,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn
	);
	if (!SpawnedActor)
	{
		DebugMessage(
			FString::Printf(TEXT("MeiDou Spawn failed: %s"), *GetNameSafe(SpawnClass)),
			FColor::Red,
			2.5f
		);
		return;
	}

	const bool bUseEndActor = (TargetActor != nullptr);

	if (SpawnedActor->GetClass()->ImplementsInterface(UMeiDouSpawnConfigurable::StaticClass()))
	{
		IMeiDouSpawnConfigurable::Execute_ConfigureMeiDouSpawn(
			SpawnedActor,
			OwnerActor,
			TargetActor,
			EndLocation,
			bUseEndActor
		);
	}

	SpawnedActor->FinishSpawning(SpawnTransform);
	DebugMessage(FString::Printf(TEXT("MeiDou Spawned: %s"), *GetNameSafe(SpawnedActor)), FColor::Green);

	SpawnedResultActor = SpawnedActor;
	RunSpawnedResultActors.RemoveAll([](const TWeakObjectPtr<AActor>& ResultEntry) { return !ResultEntry.IsValid(); });
	RunSpawnedResultActors.Add(SpawnedActor);
	bDestroySpawnedResultActorOnActionEnd = SpawnConfig.bDestroyWhenPoseActiveEnds;
}

FVector UMeiDouComponent::GetSpawnLocationForConfig(const FMeiDouSpawnConfig& SpawnConfig, AActor*& OutTargetActor)
{
	OutTargetActor = GetCurrentLockOnTarget();

	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return FVector::ZeroVector;
	}

	if (SpawnConfig.SpawnLocation == EMeiDouResultSpawnLocation::EMDRSL_Target)
	{
		if (OutTargetActor)
		{
			return OutTargetActor->GetActorLocation();
		}

		return OwnerActor->GetActorLocation() + OwnerActor->GetActorForwardVector() * SpawnConfig.ForwardDistanceIfNoTarget;
	}

	if (ACharacter* OwnerCharacter = Cast<ACharacter>(OwnerActor))
	{
		if (USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh())
		{
			if (SpawnConfig.EmittingActorSocketName != NAME_None &&
				Mesh->DoesSocketExist(SpawnConfig.EmittingActorSocketName))
			{
				return Mesh->GetSocketLocation(SpawnConfig.EmittingActorSocketName) + SpawnConfig.SpawnOffset;
			}
		}
	}

	return OwnerActor->GetActorLocation() + SpawnConfig.SpawnOffset;
}

AActor* UMeiDouComponent::GetCurrentLockOnTarget()
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return nullptr;
	}

	if (ULockOnComponent* LockOnComponent = OwnerActor->FindComponentByClass<ULockOnComponent>())
	{
		if (LockOnComponent->IsLockedOn())
		{
			return LockOnComponent->GetCurrentTarget();
		}
	}

	return nullptr;
}
