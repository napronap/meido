// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorComponents/AttackComponent.h"
#include "Animation/AnimMontage.h"
#include "Characters/EnemyMaid.h"
#include "Characters/PlayerMaidCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystems/CombatFeedbackSubsystem.h"

UAttackComponent::UAttackComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	// ticks only while a hit window is open
	PrimaryComponentTick.bStartWithTickEnabled = false;

	HandleStepMontageBlendingOut.BindUObject(this, &UAttackComponent::HandleStepMontageEnded);
}

void UAttackComponent::BeginPlay()
{
	Super::BeginPlay();

	// players and enemies
	OwnerCharacter = CastChecked<ACharacter>(GetOwner());
}

void UAttackComponent::RequestAttack(const FVector& InAimDirection)
{
	// not attacking will most likely mean I'm idle (MaidCharacter already approved)
	if (!IsAttacking())
	{
		AimDirection = InAimDirection;
		PlayStep(0);
		return;
	}

	// attacking + can still chain (didn't hit recovery)
	if (!bRecovering)
	{
		bAttackQueued = true;
		AimDirection = InAimDirection;
	}
}

void UAttackComponent::OpenComboHitWindow(const UAnimSequenceBase* Animation)
{
	if (IsCurrentStep(Animation))
	{
		OpenHitWindow(ComboData->Steps[CurrentStep].Hit);
	}
}

void UAttackComponent::CheckCombo(const UAnimSequenceBase* Animation)
{
	if (!IsCurrentStep(Animation))
	{
		return;
	}

	if (bAttackQueued && ComboData->Steps.IsValidIndex(CurrentStep + 1))
	{
		PlayStep(CurrentStep + 1);
		return;
	}

	// no input since the step started (or it was the last one)
	bRecovering = true;
	bAttackQueued = false;
}

void UAttackComponent::EndRecovery(const UAnimSequenceBase* Animation)
{
	if (IsCurrentStep(Animation))
	{
		OwnerCharacter->GetMesh()->GetAnimInstance()->Montage_Stop(0.3f, ComboData->Steps[CurrentStep].Montage);
	}
}

void UAttackComponent::PlayStep(const int32 StepIndex)
{
	CurrentStep = StepIndex;
	bAttackQueued = false;
	bRecovering = false;

	UAnimMontage* Montage = ComboData->Steps[StepIndex].Montage;
	UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();

	// snaps before the montage so its root motion already goes that way
	if (!AimDirection.IsNearlyZero())
	{
		OwnerCharacter->SetActorRotation(FRotator(0.f, AimDirection.Rotation().Yaw, 0.f));
	}

	AnimInstance->Montage_Play(Montage);
	AnimInstance->Montage_SetBlendingOutDelegate(HandleStepMontageBlendingOut, Montage);
}

void UAttackComponent::HandleStepMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	// chaining interrupts the previous step's montage; only the current step ending (recovery, damage, dash...) ends the combo
	if (!IsCurrentStep(Montage))
	{
		return;
	}

	CurrentStep = INDEX_NONE;
	bAttackQueued = false;
	bRecovering = false;
}

bool UAttackComponent::IsCurrentStep(const UAnimSequenceBase* Animation) const
{
	return IsAttacking() && ComboData->Steps[CurrentStep].Montage.Get() == Animation;
}

void UAttackComponent::OpenHitWindow(const FAttackHit& Hit)
{
	CurrentHit = Hit;
	HitActorsThisWindow.Reset();

	// first sweep starts where each socket is when the window opens
	USkeletalMeshComponent* OwnerMesh = OwnerCharacter->GetMesh();
	PreviousSocketLocations.Reset(Hit.Sockets.Num());
	for (const FName SocketName : Hit.Sockets)
	{
		PreviousSocketLocations.Add(OwnerMesh->GetSocketLocation(SocketName));
	}

	SetComponentTickEnabled(true);
}

void UAttackComponent::CloseHitWindow()
{
	SetComponentTickEnabled(false);
}

void UAttackComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction
)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	PerformHitTrace();
}

void UAttackComponent::PerformHitTrace()
{
	USkeletalMeshComponent* OwnerMesh = OwnerCharacter->GetMesh();
	UWorld* World = GetWorld();

	const FCollisionShape Box = FCollisionShape::MakeBox(CurrentHit.BoxExtent);
	const FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(AttackTrace), false, OwnerCharacter);

	for (int32 i = 0; i < CurrentHit.Sockets.Num(); ++i)
	{
		// sweep the path the box actually traveled since last frame
		// apparently skipping frames is called tunneling
		const FVector Start = PreviousSocketLocations[i];
		const FVector End = OwnerMesh->GetSocketLocation(CurrentHit.Sockets[i]);
		// previous frame's box will keep this frame's rotation but whatever
		const FQuat Rotation = OwnerMesh->GetSocketQuaternion(CurrentHit.Sockets[i]);
		PreviousSocketLocations[i] = End;

		DrawDebugBox(World, Start, CurrentHit.BoxExtent, Rotation, FColor::Orange, false, .1f);
		DrawDebugBox(World, End, CurrentHit.BoxExtent, Rotation, FColor::Red, false, .1f);

		FHitResult Hit;

		// ObjectType works for now since in this game only pawns hit other pawns
		// if I ever need to hit another thing it may be better to consider creating a channel
		if (!World->SweepSingleByObjectType(Hit, Start, End, Rotation, ECC_Pawn, Box, QueryParams))
		{
			continue;
		}

		AActor* HitActor = Hit.GetActor();
		if (HitActorsThisWindow.Contains(HitActor) || ShouldIgnoreHitActor(HitActor))
		{
			continue;
		}

		HitActorsThisWindow.Add(HitActor);

		const float AppliedDamage = UGameplayStatics::ApplyDamage(
			HitActor,
			CurrentHit.Damage,
			OwnerCharacter->GetController(),
			OwnerCharacter,
			nullptr
		);

		// rejected hits (iframes, dead, etc)
		if (AppliedDamage <= 0.f)
		{
			continue;
		}

		const float HitStopDuration = CurrentHit.HitStopType == EHitStopType::Heavy
			? HeavyHitStopDuration
			: LightHitStopDuration;

		UCombatFeedbackSubsystem* CombatFeedback = World->GetSubsystem<UCombatFeedbackSubsystem>();
		CombatFeedback->ApplyHitStop(OwnerCharacter, HitStopDuration);
		CombatFeedback->ApplyHitStop(HitActor, HitStopDuration);
	}
}

bool UAttackComponent::ShouldIgnoreHitActor(const AActor* HitActor) const
{
	if (OwnerCharacter->IsA<AEnemyMaid>())
	{
		return !HitActor->IsA<APlayerMaidCharacter>();
	}

	if (OwnerCharacter->IsA<APlayerMaidCharacter>())
	{
		return !HitActor->IsA<AEnemyMaid>();
	}

	return true;
}
