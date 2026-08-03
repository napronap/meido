// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorComponents/DashComponent.h"
#include "AnimInstances/MaidAnimInstance.h"
#include "Characters/MaidCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

UDashComponent::UDashComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UDashComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerMaid = Cast<AMaidCharacter>(GetOwner());
	if (OwnerMaid.IsValid())
	{
		MovementComponent = OwnerMaid->GetCharacterMovement();
	}
}

void UDashComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction
)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	DashCooldownRemaining = FMath::Max(0.f, DashCooldownRemaining - DeltaTime);

	if (!bIsDashing)
	{
		return;
	}

	if (!OwnerMaid.IsValid() || !MovementComponent.IsValid())
	{
		EndDash();
		return;
	}

	// No velocity override — CMC consumes anim root motion while dash anim plays.
	DashTimeRemaining -= DeltaTime;
	if (DashTimeRemaining <= 0.f)
	{
		EndDash();
	}
}

bool UDashComponent::TryDash(
	const FVector2D& MoveInput,
	const FRotator& ControlRotation,
	const bool bLockOnActive
)
{
	if (!CanDash())
	{
		return false;
	}

	FVector DashWorldDirection = FVector::ZeroVector;
	FVector2D DashAnimDirection = FVector2D(0.f, 1.f);

	if (bLockOnActive)
	{
		DashWorldDirection = ComputeDashWorldDirection(MoveInput, ControlRotation);
		DashAnimDirection = ComputeDashAnimDirection(DashWorldDirection, ControlRotation);
	}
	else
	{
		DashWorldDirection = ComputeDashWorldDirection(MoveInput, ControlRotation);
		if (DashWorldDirection.IsNearlyZero())
		{
			DashWorldDirection = OwnerMaid->GetActorForwardVector();
			DashWorldDirection.Z = 0.f;
			DashWorldDirection.Normalize();
		}

		if (!DashWorldDirection.IsNearlyZero())
		{
			const FRotator CurrentRotation = OwnerMaid->GetActorRotation();
			const FRotator DashRotation(0.f, DashWorldDirection.Rotation().Yaw, 0.f);
			OwnerMaid->SetActorRotation(
				FRotator(CurrentRotation.Pitch, DashRotation.Yaw, CurrentRotation.Roll)
			);
		}

		DashAnimDirection = FVector2D(0.f, 1.f);
	}

	if (DashWorldDirection.IsNearlyZero())
	{
		return false;
	}

	StartDash(DashWorldDirection, DashAnimDirection);
	return true;
}

void UDashComponent::CancelDash()
{
	if (!bIsDashing)
	{
		return;
	}

	EndDash();
}

void UDashComponent::NotifyDashAnimationEnded()
{
	if (bIsDashing)
	{
		EndDash();
	}
}

bool UDashComponent::CanDash() const
{
	if (bIsDashing || DashCooldownRemaining > 0.f || !OwnerMaid.IsValid() || !MovementComponent.IsValid())
	{
		return false;
	}

	if (!OwnerMaid->CanStartDash())
	{
		return false;
	}

	if (MovementComponent->IsFalling())
	{
		return false;
	}

	return true;
}

FVector UDashComponent::ComputeDashWorldDirection(
	const FVector2D& MoveInput,
	const FRotator& ControlRotation
) const
{
	FVector2D DashInput = MoveInput;
	if (DashInput.SizeSquared() < FMath::Square(MinDirectionInput))
	{
		DashInput = FVector2D(0.f, 1.f);
	}
	else
	{
		DashInput.Normalize();
	}

	const FRotator YawRotation(0.f, ControlRotation.Yaw, 0.f);
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	FVector DashWorldDirection = (ForwardDirection * DashInput.Y) + (RightDirection * DashInput.X);
	DashWorldDirection.Z = 0.f;
	DashWorldDirection.Normalize();

	return DashWorldDirection;
}

FVector2D UDashComponent::ComputeDashAnimDirection(
	const FVector& WorldDirection,
	const FRotator& ControlRotation
) const
{
	const FRotator YawRotation(0.f, ControlRotation.Yaw, 0.f);
	const FVector LocalDirection = UKismetMathLibrary::InverseTransformDirection(
		FTransform(YawRotation, FVector::ZeroVector, FVector::OneVector),
		WorldDirection
	);

	return FVector2D(
		FMath::Clamp(LocalDirection.Y, -1.f, 1.f),
		FMath::Clamp(LocalDirection.X, -1.f, 1.f)
	);
}

void UDashComponent::StartDash(const FVector& InDashDirection, const FVector2D& AnimDirection)
{
	if (!OwnerMaid.IsValid() || !MovementComponent.IsValid())
	{
		return;
	}

	(void)InDashDirection;

	DashTimeRemaining = DashDuration;
	// Cooldown starts on End (CP2.1) — not here.
	bIsDashing = true;

	MovementComponent->StopMovementImmediately();

	bSavedOrientRotationToMovement = MovementComponent->bOrientRotationToMovement;
	MovementComponent->bOrientRotationToMovement = false;

	SavedAnimRootMotionScale = OwnerMaid->GetAnimRootMotionTranslationScale();
	OwnerMaid->SetAnimRootMotionTranslationScale(DashRootMotionScale);

	// Allow state-machine / BS dash RM while ABP default is Montages Only.
	ApplyDashRootMotionMode();

	UpdateDashAnimState(true, AnimDirection);
	OwnerMaid->NotifyDashStarted(bUseLegacyTimedIFramesOnStart);
}

void UDashComponent::EndDash()
{
	const bool bWasDashing = bIsDashing;
	bIsDashing = false;
	DashTimeRemaining = 0.f;

	if (bWasDashing)
	{
		// Cooldown starts when dash ends.
		DashCooldownRemaining = DashCooldown;
	}

	// Leave dash pose before restoring Montages Only so loco doesn't keep feeding RM.
	UpdateDashAnimState(false, FVector2D::ZeroVector);
	RestoreRootMotionMode();

	if (OwnerMaid.IsValid())
	{
		OwnerMaid->SetAnimRootMotionTranslationScale(SavedAnimRootMotionScale);
	}

	if (MovementComponent.IsValid())
	{
		MovementComponent->bOrientRotationToMovement = bSavedOrientRotationToMovement;
		MovementComponent->StopMovementImmediately();
	}

	if (OwnerMaid.IsValid() && bWasDashing)
	{
		OwnerMaid->NotifyDashEnded();
	}
}

UAnimInstance* UDashComponent::GetOwnerAnimInstance() const
{
	if (!OwnerMaid.IsValid() || !OwnerMaid->GetMesh())
	{
		return nullptr;
	}

	return OwnerMaid->GetMesh()->GetAnimInstance();
}

void UDashComponent::ApplyDashRootMotionMode()
{
	bRootMotionModeOverridden = false;

	if (!bForceRootMotionFromEverythingWhileDashing)
	{
		return;
	}

	UAnimInstance* AnimInstance = GetOwnerAnimInstance();
	if (!AnimInstance)
	{
		return;
	}

	// RootMotionMode is a public UPROPERTY; no getter in UE 5.6.
	SavedRootMotionMode = AnimInstance->RootMotionMode;
	AnimInstance->SetRootMotionMode(ERootMotionMode::RootMotionFromEverything);
	bRootMotionModeOverridden = true;
}

void UDashComponent::RestoreRootMotionMode()
{
	if (!bRootMotionModeOverridden)
	{
		return;
	}

	if (UAnimInstance* AnimInstance = GetOwnerAnimInstance())
	{
		AnimInstance->SetRootMotionMode(SavedRootMotionMode);
	}

	bRootMotionModeOverridden = false;
}

void UDashComponent::UpdateDashAnimState(const bool bDashing, const FVector2D& AnimDirection) const
{
	if (!OwnerMaid.IsValid() || !OwnerMaid->GetMesh())
	{
		return;
	}

	UMaidAnimInstance* MaidAnimInstance = Cast<UMaidAnimInstance>(OwnerMaid->GetMesh()->GetAnimInstance());
	if (!MaidAnimInstance)
	{
		return;
	}

	MaidAnimInstance->bIsDashing = bDashing;
	MaidAnimInstance->DashRight = bDashing ? AnimDirection.X : 0.f;
	MaidAnimInstance->DashForward = bDashing ? AnimDirection.Y : 0.f;
}
