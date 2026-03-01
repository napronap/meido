// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorComponents/MeiDouComponent.h"
#include "ActorComponents/LockOnComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Interfaces/MeiDouSpawnConfigurable.h"
#include "TimerManager.h"

// Sets default values for this component's properties
UMeiDouComponent::UMeiDouComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


const TArray<EMeiDouInput>& UMeiDouComponent::GetInputBuffer() const
{
	return ComboInputBuffer;
}

int32 UMeiDouComponent::GetInputCount(EMeiDouInput Input) const
{
	int32 Count = 0;

	for (int32 i = 0; i < ComboInputBuffer.Num(); i++)
	{
		if (ComboInputBuffer[i] == Input)
		{
			Count++;
		}
		else
		{
			break;
		}
	}
	
	return Count;
}

void UMeiDouComponent::BeginPlay()
{
	Super::BeginPlay();
	ComboInputBuffer.Reserve(MaxInputs);
	QueuedPoseInputs.Reserve(MaxInputs);
	ClearActiveComboDefinition();
	ClearPendingResolvedCombo();
	SpawnedResultActor.Reset();
	bDestroySpawnedResultActorOnActionEnd = false;
	ClearComboInputTimeout();
	SetMeiDouState(EMeiDouState::EMDS_Idle);
}

bool UMeiDouComponent::RegisterInput(EMeiDouInput Input)
{
	if (Input == EMeiDouInput::None)
	{
		return false;
	}

	if (QueuedPoseInputs.Num() >= MaxInputs)
	{
		return false;
	}

	ClearComboInputTimeout();
	QueuedPoseInputs.Add(Input);
	TryConsumeNextQueuedAction();

	return true;
}

void UMeiDouComponent::OnMeiDouActionWindowBegin()
{
	OnMeiDouControlLockChanged.Broadcast(true);

	if (HasActiveComboDefinition())
	{
		SetMeiDouState(EMeiDouState::EMDS_Finished);
		return;
	}

	SetMeiDouState(EMeiDouState::EMDS_Posing);
}

void UMeiDouComponent::OnMeiDouActionWindowEnd()
{
	OnMeiDouControlLockChanged.Broadcast(false);
	CleanupSpawnedResultActor();

	if (MeiDouState == EMeiDouState::EMDS_Finished)
	{
		ClearActiveComboDefinition();
	}

	SetMeiDouState(EMeiDouState::EMDS_Idle);
	TryConsumeNextQueuedAction();
}

void UMeiDouComponent::OnRequestedAnimationFailed()
{
	OnMeiDouControlLockChanged.Broadcast(false);
	CleanupSpawnedResultActor();

	if (MeiDouState == EMeiDouState::EMDS_Finished)
	{
		ClearActiveComboDefinition();
	}

	SetMeiDouState(EMeiDouState::EMDS_Idle);
	TryConsumeNextQueuedAction();
}

void UMeiDouComponent::ResetCombo()
{
	ComboInputBuffer.Reset();
	ClearComboInputTimeout();
}

void UMeiDouComponent::HandleAnimEvent(EMeiDouAnimEvent EventKey)
{
	switch (EventKey)
	{
	case EMeiDouAnimEvent::EMDAE_Spawn:
		if (!HasActiveComboDefinition())
		{
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("MeiDou Spawn notify ignored: no active combo"));
			}
			return;
		}

		if (ActiveComboDefinition.ResultType == EMeiDouResultType::EMDRT_Spawn)
		{
			ExecuteSpawnResult(ActiveComboDefinition.SpawnConfig);
		}
		else if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("MeiDou Spawn notify ignored: result is not Spawn"));
		}
		break;
	case EMeiDouAnimEvent::EMDAE_ControlEnable:
	case EMeiDouAnimEvent::EMDAE_ControlDisable:
	case EMeiDouAnimEvent::EMDAE_TraceStart:
	case EMeiDouAnimEvent::EMDAE_TraceEnd:
	default:
		break;
	}
}

const FMeiDouComboDefinition* UMeiDouComponent::GetActiveComboDefinition() const
{
	if (!HasActiveComboDefinition())
	{
		return nullptr;
	}

	return &ActiveComboDefinition;
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

bool UMeiDouComponent::HasActiveComboDefinition() const
{
	return bHasActiveComboDefinition;
}

bool UMeiDouComponent::HasPendingResolvedCombo() const
{
	return bHasPendingResolvedCombo;
}

void UMeiDouComponent::ClearActiveComboDefinition()
{
	ActiveComboDefinition = FMeiDouComboDefinition();
	bHasActiveComboDefinition = false;
}

void UMeiDouComponent::ClearPendingResolvedCombo()
{
	PendingResolvedComboDefinition = FMeiDouComboDefinition();
	PendingResolvedComboResult = FMeiDouResolvedCombo();
	bHasPendingResolvedCombo = false;
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

void UMeiDouComponent::RestartComboInputTimeout()
{
	if (ComboInputTimeoutSeconds <= 0.f)
	{
		return;
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
}

void UMeiDouComponent::ClearComboInputTimeout()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	World->GetTimerManager().ClearTimer(ComboInputTimeoutHandle);
}

void UMeiDouComponent::OnComboInputTimeout()
{
	if (ComboInputBuffer.Num() <= 0)
	{
		return;
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Yellow, TEXT("MeiDou combo timeout: buffer reset"));
	}

	ResetCombo();
}

void UMeiDouComponent::TryConsumeNextQueuedAction()
{
	if (MeiDouState != EMeiDouState::EMDS_Idle)
	{
		return;
	}

	if (QueuedPoseInputs.Num() > 0)
	{
		ClearComboInputTimeout();

		const EMeiDouInput NextInput = QueuedPoseInputs[0];
		QueuedPoseInputs.RemoveAt(0);

		const int32 NextInputCount = GetInputCount(NextInput) + 1;
		RegisterInputToComboBuffer(NextInput);

		FMeiDouPoseAnimationRequest Request;
		Request.Input = NextInput;
		Request.bUseMirroredMontage = (NextInputCount % 2 == 0);

		SetMeiDouState(EMeiDouState::EMDS_Posing);
		OnPoseAnimationRequested.Broadcast(Request);
		return;
	}

	if (HasPendingResolvedCombo())
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				1.5f,
				FColor::Yellow,
				TEXT("MeiDou: entering combo execution phase")
			);
		}

		ActiveComboDefinition = PendingResolvedComboDefinition;
		bHasActiveComboDefinition = true;
		const FMeiDouResolvedCombo ResolvedResult = PendingResolvedComboResult;
		ClearPendingResolvedCombo();

		SetMeiDouState(EMeiDouState::EMDS_Finished);
		OnComboResolved.Broadcast(ResolvedResult);
		return;
	}

	// Idle with a partial combo buffer waiting for the next input.
	if (ComboInputBuffer.Num() > 0)
	{
		RestartComboInputTimeout();
	}
}

bool UMeiDouComponent::RegisterInputToComboBuffer(EMeiDouInput Input)
{
	if (ComboInputBuffer.Num() >= MaxInputs)
	{
		return false;
	}

	ComboInputBuffer.Add(Input);

	if (ComboInputBuffer.Num() == MaxInputs)
	{
		return TryResolve();
	}

	return false;
}

bool UMeiDouComponent::TryResolve()
{
	if (!ComboData || ComboInputBuffer.Num() != 3)
	{
		return false;
	}

	FMeiDouComboKey Key;
	Key.A = ComboInputBuffer[0];
	Key.B = ComboInputBuffer[1];
	Key.C = ComboInputBuffer[2];

	if (const FMeiDouComboDefinition* Def = ComboData->Combos.Find(Key))
	{
		ExecuteResolvedCombo(*Def);

		PendingResolvedComboResult.ComboId = Def->ComboId;
		PendingResolvedComboResult.Inputs = ComboInputBuffer;

		if (GEngine)
		{
			const FString SuccessMessage = FString::Printf(
				TEXT("MeiDou combo SUCCESS: %s"),
				*Def->ComboId.ToString()
			);
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, SuccessMessage);
		}

		ResetCombo();
		return true;
	}

	if (GEngine)
	{
		const FString FailMessage = FString::Printf(
			TEXT("MeiDou combo FAIL: [%s, %s, %s]"),
			*StaticEnum<EMeiDouInput>()->GetNameStringByValue(static_cast<int64>(Key.A)),
			*StaticEnum<EMeiDouInput>()->GetNameStringByValue(static_cast<int64>(Key.B)),
			*StaticEnum<EMeiDouInput>()->GetNameStringByValue(static_cast<int64>(Key.C))
		);
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, FailMessage);
	}

	// if combo doesn't exist, do nothing and reset the input
	ResetCombo();
	return false;
}

void UMeiDouComponent::ExecuteResolvedCombo(const FMeiDouComboDefinition& Definition)
{
	PendingResolvedComboDefinition = Definition;
	bHasPendingResolvedCombo = true;
}

void UMeiDouComponent::ExecuteSpawnResult(const FMeiDouSpawnConfig& SpawnConfig)
{
	UWorld* World = GetWorld();
	AActor* OwnerActor = GetOwner();
	if (!World)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("MeiDou Spawn failed: invalid World"));
		}
		return;
	}

	if (!OwnerActor)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("MeiDou Spawn failed: invalid Owner"));
		}
		return;
	}

	if (!SpawnConfig.SpawnedActorClass)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.5f, FColor::Red, TEXT("MeiDou Spawn failed: SpawnedActorClass is null"));
		}
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

	AActor* SpawnedActor = World->SpawnActorDeferred<AActor>(
		SpawnConfig.SpawnedActorClass,
		SpawnTransform,
		OwnerActor,
		InstigatorPawn,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn
	);
	if (!SpawnedActor)
	{
		if (GEngine)
		{
			const FString FailMessage = FString::Printf(
				TEXT("MeiDou Spawn failed: could not defer spawn %s"),
				*GetNameSafe(SpawnConfig.SpawnedActorClass.Get())
			);
			GEngine->AddOnScreenDebugMessage(-1, 2.5f, FColor::Red, FailMessage);
		}
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
	if (GEngine)
	{
		const FString SuccessMessage = FString::Printf(
			TEXT("MeiDou Spawned: %s"),
			*GetNameSafe(SpawnedActor)
		);
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, SuccessMessage);
	}

	SpawnedResultActor = SpawnedActor;
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
