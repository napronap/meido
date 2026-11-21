// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorComponents/DashComponent.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UDashComponent::UDashComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	OnDashMontageBlendingOut.BindUObject(this, &UDashComponent::HandleDashMontageBlendingOut);
}

void UDashComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = CastChecked<ACharacter>(GetOwner());
}

bool UDashComponent::TryDash(const FVector2D& MoveInput, const FRotator& ControlRotation, const bool bLockOnActive)
{
	if (bIsDashing)
	{
		return false;
	}

	const bool bHasInput = MoveInput.SizeSquared() >= FMath::Square(MinDirectionInput);

	// stick (X right, Y forward) to world, using the camera's yaw (a dash is basically always "forward" in terms of animation)
	const FRotator Yaw(0.f, ControlRotation.Yaw, 0.f);
	const FVector WorldDirection = Yaw.RotateVector(FVector(MoveInput.Y, MoveInput.X, 0.f)).GetSafeNormal();

	// free movement: root motion goes where the character faces, so face the input
	if (bHasInput && !bLockOnActive)
	{
		// very important!! set rotation so the character snaps to the input,
		// otherwise the dash will get caught mid rotation and not launch on the intended direction 
		OwnerCharacter->SetActorRotation(FRotator(0.f, WorldDirection.Rotation().Yaw, 0.f));
	}

	UAnimMontage* Montage = PickMontage(WorldDirection, bHasInput, bLockOnActive);

	bIsDashing = true;
	OwnerCharacter->GetCharacterMovement()->StopMovementImmediately();

	UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
	AnimInstance->Montage_Play(Montage);
	AnimInstance->Montage_SetBlendingOutDelegate(OnDashMontageBlendingOut, Montage);

	return true;
}

UAnimMontage* UDashComponent::PickMontage(const FVector& WorldDirection, const bool bHasInput, const bool bLockOnActive) const
{
	// dash back from facing direction if there's no input
	if (!bHasInput)
	{
		return BackwardMontage;
	}

	// has input and no lockon: always forward
	if (!bLockOnActive)
	{
		return ForwardMontage;
	}

	// lockon keeps character facing the target (controller yaw)
	// pick the montage from the relative input
	// local direction (X forward, Y right)
	const FVector LocalDirection = OwnerCharacter->GetActorTransform().InverseTransformVectorNoScale(WorldDirection);

	// |x| >= |y| means the input is forwards leaning
	if (FMath::Abs(LocalDirection.X) >= FMath::Abs(LocalDirection.Y))
	{
		return LocalDirection.X >= 0.f ? ForwardMontage : BackwardMontage;
	}

	// if neither forwards nor backwards, then check |y| for horizontal direction
	return LocalDirection.Y >= 0.f ? RightMontage : LeftMontage;
}

void UDashComponent::HandleDashMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted)
{
	bIsDashing = false;
	OwnerCharacter->GetCharacterMovement()->StopMovementImmediately();
}
