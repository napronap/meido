// Fill out your copyright notice in the Description page of Project Settings.

#include "AnimInstances/MaidAnimInstance.h"

#include "ActorComponents/CharacterStateComponent.h"
#include "ActorComponents/LockOnComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

void UMaidAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
}

void UMaidAnimInstance::NativeUpdateAnimation(const float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!OwnerCharacter)
	{
		OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
	}

	ACharacter* const Character = OwnerCharacter.Get();
	if (!Character)
	{
		Velocity = FVector::ZeroVector;
		GroundSpeed = 0.f;
		ForwardSpeed = 0.f;
		RightSpeed = 0.f;
		bShouldMove = false;
		bIsFalling = false;
		bIsInWater = false;
		return;
	}

	// --- Loco: same math as ABP_Manny_Maid EventGraph (docs/refs/Maid_ABP_Ref) ---
	if (const UCharacterMovementComponent* const Move = Character->GetCharacterMovement())
	{
		Velocity = Move->Velocity;
		GroundSpeed = Velocity.Size2D(); // VSizeXY
		bIsFalling = Move->IsFalling();
		bIsInWater = Move->IsSwimming();

		const FVector Forward = Character->GetActorForwardVector();
		const FVector Right = Character->GetActorRightVector();
		ForwardSpeed = FVector::DotProduct(Velocity, Forward);
		RightSpeed = FVector::DotProduct(Velocity, Right);

		// Pack: GroundSpeed > 3 AND CurrentAcceleration != 0
		const bool bHasAccel = !Move->GetCurrentAcceleration().IsNearlyZero();
		bShouldMove = (GroundSpeed > ShouldMoveSpeedThreshold) && bHasAccel;
	}
	else
	{
		Velocity = FVector::ZeroVector;
		GroundSpeed = 0.f;
		ForwardSpeed = 0.f;
		RightSpeed = 0.f;
		bShouldMove = false;
		bIsFalling = false;
		bIsInWater = false;
	}

	// --- State copies ---
	if (const UCharacterStateComponent* const State =
		Character->FindComponentByClass<UCharacterStateComponent>())
	{
		OverallState = State->GetOverall();
		bIsDead = State->IsDead();
		bIsAttacking = State->IsAttacking();
		bIsStaggered = State->IsStaggered();
		bIsMeiDouActive = State->IsMeiDouLocked();
		// DashForward/Right stay event-pushed by DashComponent.
		bIsDashing = State->IsDashing();
	}

	if (ULockOnComponent* const LockOn = Character->FindComponentByClass<ULockOnComponent>())
	{
		bIsLockedOn = LockOn->IsLockedOn();
	}
}
