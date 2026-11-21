// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorComponents/MeiDouComponent.h"
#include "ActorComponents/AttackComponent.h"
#include "ActorComponents/LockOnComponent.h"
#include "AnimInstances/MaidAnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Interfaces/MeiDouSpawnConfigurable.h"
#include "TimerManager.h"

UMeiDouComponent::UMeiDouComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UMeiDouComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = CastChecked<ACharacter>(GetOwner());
	AttackComponent = OwnerCharacter->FindComponentByClass<UAttackComponent>();
	LockOnComponent = OwnerCharacter->FindComponentByClass<ULockOnComponent>();

	ComboInputBuffer.Reserve(3);
}

void UMeiDouComponent::RegisterInput(const EMeiDouInput Input)
{
	switch (State)
	{
	case EMeiDouState::EMDS_Idle:
		PlayPose(Input);
		return;

	case EMeiDouState::EMDS_Posing:
		// if the 3rd pose is already playing, what comes next is the result
		if (ComboInputBuffer.Num() < 3 && QueuedInput == EMeiDouInput::None)
		{
			QueuedInput = Input;
		}
		return;

	default:
		// result or fail in progress
		return;
	}
}

void UMeiDouComponent::PlayPose(const EMeiDouInput Input)
{
	GetWorld()->GetTimerManager().ClearTimer(ComboInputTimeoutHandle);

	const bool bMirror = ComboInputBuffer.Num() > 0 && ComboInputBuffer.Last() == Input && !bLastPoseMirrored;
	bLastPoseMirrored = bMirror;
	SetMirrorFlag(bMirror);

	ComboInputBuffer.Add(Input);
	State = EMeiDouState::EMDS_Posing;
	
	const FMeiDouPose& Pose = PoseSet->GetPose(Input);
	PlayMontage(Pose.Montage, Pose.PlayRate, TEXT("Pose"));
}

void UMeiDouComponent::Resolve()
{
	// searches for enum combinations i.e 001 == MMK
	const FMeiDouCombo* Combo = ComboData->Combos.FindByPredicate(
		[this](const FMeiDouCombo& Candidate) { return Candidate.Inputs == ComboInputBuffer; });

	ResetCombo();

	if (!Combo)
	{
		State = EMeiDouState::EMDS_Failed;
		PlayMontage(PoseSet->FailMontage, 1.f);
		return;
	}

	ActiveCombo = *Combo;
	State = EMeiDouState::EMDS_Result;
	PlayMontage(Combo->Montage, 1.f);
}

void UMeiDouComponent::HandleAnimEvent(const EMeiDouAnimEvent Event, const UAnimSequenceBase* Animation)
{
	if (Animation != CurrentMontage.Get())
	{
		return;
	}

	switch (Event)
	{
	case EMeiDouAnimEvent::EMDAE_ActionEnd:
		// character is playing a pose AND triggers action end notify (reached the end of the current one before state update)
		if (State == EMeiDouState::EMDS_Posing)
		{
			// reaches end of third input, time to resolve 
			if (ComboInputBuffer.Num() == 3)
			{
				Resolve();
				return;
			}

			// there's queued inputs
			if (QueuedInput != EMeiDouInput::None)
			{
				const EMeiDouInput NextInput = QueuedInput;
				QueuedInput = EMeiDouInput::None;
				PlayPose(NextInput);
				return;
			}

			// the pose montage keeps playing its "return" part
			State = EMeiDouState::EMDS_Idle;
			SetMirrorFlag(false);

			// start the combo timeout timer
			GetWorld()->GetTimerManager().SetTimer(
				ComboInputTimeoutHandle,
				this,
				&UMeiDouComponent::ResetCombo,
				ComboInputTimeoutSeconds,
				false
			);
			return;
		}

		if (State == EMeiDouState::EMDS_Result)
		{
			EndAction();
		}
		return;

	// moment the spawn animation should actually spawn the thing
	case EMeiDouAnimEvent::EMDAE_Spawn:
		ExecuteSpawnResult(ActiveCombo.Spawn);
		return;

	// moment in which tracing should start/end for Hit types
	case EMeiDouAnimEvent::EMDAE_TraceStart:
		AttackComponent->OpenHitWindow(ActiveCombo.Hit);
		return;
	
	case EMeiDouAnimEvent::EMDAE_TraceEnd:
		AttackComponent->CloseHitWindow();
	}
}

void UMeiDouComponent::EndAction()
{
	AttackComponent->CloseHitWindow();
	// TODO: arguably spawned actor should clean up on its own
	CleanupSpawnedResultActor();
	ResetCombo();
	ActiveCombo = FMeiDouCombo();
	State = EMeiDouState::EMDS_Idle;
}

void UMeiDouComponent::ResetCombo()
{
	GetWorld()->GetTimerManager().ClearTimer(ComboInputTimeoutHandle);
	ComboInputBuffer.Reset();
	QueuedInput = EMeiDouInput::None;
	bLastPoseMirrored = false;
	SetMirrorFlag(false);
}

void UMeiDouComponent::PlayMontage(UAnimMontage* Montage, const float PlayRate, const FName Section)
{
	CurrentMontage = Montage;
	
	OwnerCharacter->GetCharacterMovement()->StopMovementImmediately();
	OwnerCharacter->PlayAnimMontage(Montage, PlayRate, Section);

	// store the instance ID of the montage we just played
	UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
	CurrentInstanceId = AnimInstance->GetActiveInstanceForMontage(Montage)->GetInstanceID();
	
	FOnMontageBlendingOutStarted OnBlendingOut = FOnMontageBlendingOutStarted::CreateUObject(
		this,
		&UMeiDouComponent::HandleMontageBlendingOut,
		CurrentInstanceId
	);
	AnimInstance->Montage_SetBlendingOutDelegate(OnBlendingOut, Montage);
}

void UMeiDouComponent::HandleMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted, const int32 InstanceId)
{
	// this is only important if a blendout from another MeiDou related event did not happen
	// if the payload's id is the same as the currently playing one, it means that a pose ended on its own,
	// and therefore we need to process it
	// otherwise, if the pose is different, it means that a new MeiDou related montage started
	// (nothing to do, everything else is managed by the component) 
	if (InstanceId == CurrentInstanceId)
	{
		CurrentMontage = nullptr;
		CurrentInstanceId = INDEX_NONE;

		switch (State)
		{
		// the fail montage just ended
		case EMeiDouState::EMDS_Failed:
			State = EMeiDouState::EMDS_Idle;
			break;

		// any other normal pose ended (flow reset or death?)
		case EMeiDouState::EMDS_Posing:
		case EMeiDouState::EMDS_Result:
			EndAction();
			break;

		default:
			// a pose that already reached ActionEnd finishing its "return" part
			break;
		}
	}
}

void UMeiDouComponent::SetMirrorFlag(const bool bShouldMirror) const
{
	CastChecked<UMaidAnimInstance>(OwnerCharacter->GetMesh()->GetAnimInstance())->bShouldMirror = bShouldMirror;
}

// TODO: fix the spawn logic
void UMeiDouComponent::ExecuteSpawnResult(const FMeiDouSpawnConfig& SpawnConfig)
{
	AActor* TargetActor = nullptr;
	const FVector SpawnLocation = GetSpawnLocationForConfig(SpawnConfig, TargetActor);
	const FVector EndLocation = TargetActor
		? TargetActor->GetActorLocation()
		: OwnerCharacter->GetActorLocation() + OwnerCharacter->GetActorForwardVector() * SpawnConfig.ForwardDistanceIfNoTarget;
	const FTransform SpawnTransform(OwnerCharacter->GetActorRotation(), SpawnLocation);

	AActor* SpawnedActor = GetWorld()->SpawnActorDeferred<AActor>(
		SpawnConfig.SpawnedActorClass,
		SpawnTransform,
		OwnerCharacter,
		OwnerCharacter,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn
	);

	if (SpawnedActor->GetClass()->ImplementsInterface(UMeiDouSpawnConfigurable::StaticClass()))
	{
		IMeiDouSpawnConfigurable::Execute_ConfigureMeiDouSpawn(
			SpawnedActor,
			OwnerCharacter,
			TargetActor,
			EndLocation,
			TargetActor != nullptr
		);
	}

	SpawnedActor->FinishSpawning(SpawnTransform);

	SpawnedResultActor = SpawnedActor;
	bDestroySpawnedResultActorOnActionEnd = SpawnConfig.bDestroyWhenPoseActiveEnds;
}

FVector UMeiDouComponent::GetSpawnLocationForConfig(const FMeiDouSpawnConfig& SpawnConfig, AActor*& OutTargetActor) const
{
	OutTargetActor = LockOnComponent->IsLockedOn() ? LockOnComponent->GetCurrentTarget() : nullptr;

	if (SpawnConfig.SpawnLocation == EMeiDouResultSpawnLocation::EMDRSL_Target)
	{
		return OutTargetActor
			? OutTargetActor->GetActorLocation()
			: OwnerCharacter->GetActorLocation() + OwnerCharacter->GetActorForwardVector() * SpawnConfig.ForwardDistanceIfNoTarget;
	}

	return OwnerCharacter->GetMesh()->GetSocketLocation(SpawnConfig.EmittingActorSocketName) + SpawnConfig.SpawnOffset;
}

void UMeiDouComponent::CleanupSpawnedResultActor()
{
	if (AActor* SpawnedActor = SpawnedResultActor.Get())
	{
		if (SpawnedActor->GetClass()->ImplementsInterface(UMeiDouSpawnConfigurable::StaticClass()))
		{
			IMeiDouSpawnConfigurable::Execute_OnMeiDouActionEnded(SpawnedActor);
		}

		if (bDestroySpawnedResultActorOnActionEnd)
		{
			SpawnedActor->Destroy();
		}
	}

	SpawnedResultActor.Reset();
	bDestroySpawnedResultActorOnActionEnd = false;
}
