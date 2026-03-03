// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorComponents/DashComponent.h"
#include "AnimInstances/MaidAnimInstance.h"
#include "Characters/MaidCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values for this component's properties
UDashComponent::UDashComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UDashComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerMaid = Cast<AMaidCharacter>(GetOwner());
	if (OwnerMaid.IsValid())
	{
		MovementComponent = OwnerMaid->GetCharacterMovement();
	}
}


// Called every frame
void UDashComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
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

	DashTimeRemaining -= DeltaTime;
	if (DashTimeRemaining <= 0.f)
	{
		EndDash();
		return;
	}

	MovementComponent->Velocity = DashDirection * DashSpeed;
}

bool UDashComponent::TryDash(const FVector2D& MoveInput, const FRotator& ControlRotation, bool bLockOnActive)
{
	if (!CanDash())
	{
		return false;
	}

	FVector DashWorldDirection = FVector::ZeroVector;
	FVector2D DashAnimDirection = FVector2D(0.f, 1.f);

	if (bLockOnActive)
	{
		// Lock-on: dash direction follows input (including diagonals).
		DashWorldDirection = ComputeDashWorldDirection(MoveInput, ControlRotation);
		DashAnimDirection = ComputeDashAnimDirection(DashWorldDirection, ControlRotation);
	}
	else
	{
		// Free mode: dash follows movement input, but character rotates to face dash
		// direction so the dash anim can stay "forward".
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
			OwnerMaid->SetActorRotation(FRotator(CurrentRotation.Pitch, DashRotation.Yaw, CurrentRotation.Roll));
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

FVector UDashComponent::ComputeDashWorldDirection(const FVector2D& MoveInput, const FRotator& ControlRotation) const
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

FVector2D UDashComponent::ComputeDashAnimDirection(const FVector& WorldDirection, const FRotator& ControlRotation) const
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

	DashDirection = InDashDirection.GetSafeNormal2D();
	DashTimeRemaining = DashDuration;
	DashCooldownRemaining = DashCooldown;
	bIsDashing = true;

	MovementComponent->StopMovementImmediately();
	UpdateDashAnimState(true, AnimDirection);
	OwnerMaid->NotifyDashStarted();
}

void UDashComponent::EndDash()
{
	if (!OwnerMaid.IsValid() || !MovementComponent.IsValid())
	{
		bIsDashing = false;
		UpdateDashAnimState(false, FVector2D::ZeroVector);
		return;
	}

	bIsDashing = false;
	DashTimeRemaining = 0.f;
	MovementComponent->StopMovementImmediately();
	UpdateDashAnimState(false, FVector2D::ZeroVector);
	OwnerMaid->NotifyDashEnded();
}

void UDashComponent::UpdateDashAnimState(bool bDashing, const FVector2D& AnimDirection) const
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

