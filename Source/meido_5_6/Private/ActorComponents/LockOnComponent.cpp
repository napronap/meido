// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorComponents/LockOnComponent.h"
#include "Algo/MinElement.h"
#include "AnimInstances/MaidAnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Interfaces/Targetable.h"
#include "Kismet/KismetSystemLibrary.h"

ULockOnComponent::ULockOnComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void ULockOnComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = CastChecked<ACharacter>(GetOwner());
}

bool ULockOnComponent::TryLockOn()
{
	const TArray<FScreenTarget> Targets = GetOnScreenTargets();
	const FScreenTarget* Closest = Algo::MinElementBy(Targets,
		[](const FScreenTarget& Target) { return Target.ScreenOffset.SizeSquared(); });

	const bool bLocked = Closest != nullptr;
	SetTarget(bLocked ? Closest->Actor : nullptr);
	ApplyLockOnState(bLocked);

	// actually gates the tick
	return bLocked;
}

void ULockOnComponent::ClearLockOn()
{
	SetTarget(nullptr);
	ApplyLockOnState(false);
}

void ULockOnComponent::SwitchTarget(const int32 Direction)
{
	// on screen targets from left to right
	TArray<FScreenTarget> Targets = GetOnScreenTargets();
	Targets.Sort([](const FScreenTarget& A, const FScreenTarget& B) { return A.ScreenOffset.X < B.ScreenOffset.X; });

	// not locked, or the current target is off screen
	const int32 CurrentIndex = Targets.IndexOfByPredicate(
		[this](const FScreenTarget& Target) { return Target.Actor == CurrentTarget; });
	if (CurrentIndex == INDEX_NONE)
	{
		return;
	}

	// the neighbor in Direction and  wrapping around the edges
	AActor* NextTarget = Targets[(CurrentIndex + Direction + Targets.Num()) % Targets.Num()].Actor;
	if (NextTarget != CurrentTarget)
	{
		SetTarget(NextTarget);
	}
}

TArray<ULockOnComponent::FScreenTarget> ULockOnComponent::GetOnScreenTargets() const
{
	TArray<AActor*> Overlaps;
	TArray<TEnumAsByte<EObjectTypeQuery>> TraceObjectTypes;
	// as always this works because all of our targetables are Pawns for now
	TraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
	
	UKismetSystemLibrary::SphereOverlapActors(
		GetWorld(),
		OwnerCharacter->GetActorLocation(),
		SearchRadius,
		TraceObjectTypes,
		nullptr,
		{OwnerCharacter.Get()},
		Overlaps
	);

	const APlayerController* PC = CastChecked<APlayerController>(OwnerCharacter->GetController());
	
	int32 ViewportX;
	int32 ViewportY;
	
	PC->GetViewportSize(ViewportX, ViewportY);
	const FVector2D HalfViewport(ViewportX * 0.5f, ViewportY * 0.5f);

	TArray<FScreenTarget> OnScreen;
	for (AActor* Actor : Overlaps)
	{
		FVector2D ScreenPosition;
		// ProjectWorldLocationToScreen returns true if the target is in front of the camera and
		// writes in ScreenPosition where it is (offset from top left corner) 
		if (IsTargetable(Actor) && PC->ProjectWorldLocationToScreen(ITargetable::Execute_GetTargetLocation(Actor), ScreenPosition))
		{
			// inside the viewport when both axes are within half of its size from the center
			// if ScreenPosition is negative or bigger than the viewport
			// it means the target is in front of the camera but far on the edges
			// ...don't think too much about it, it's just how it is
			const FVector2D Offset = ScreenPosition - HalfViewport;
			if (FMath::Abs(Offset.X) <= HalfViewport.X && FMath::Abs(Offset.Y) <= HalfViewport.Y)
			{
				OnScreen.Add({Actor, Offset});
			}
		}
	}
	return OnScreen;
}

bool ULockOnComponent::IsTargetable(AActor* Actor)
{
	return IsValid(Actor) && Actor->Implements<UTargetable>() && ITargetable::Execute_CanBeTargeted(Actor);
}

void ULockOnComponent::SetTarget(AActor* NewTarget)
{
	// TickComponent retargets when the old one is gone, so it can already be destroyed here
	// IsValid returns false when the pointer is null or the actor called Destroy() so this is the simplest way to check 
	if (IsValid(CurrentTarget))
	{
		ITargetable::Execute_SetLockedOn(CurrentTarget, false);
	}

	CurrentTarget = NewTarget;

	if (CurrentTarget)
	{
		ITargetable::Execute_SetLockedOn(CurrentTarget, true);
	}
}

void ULockOnComponent::ApplyLockOnState(const bool bLockedOn)
{
	// actually modify the character
	OwnerCharacter->bUseControllerRotationYaw = bLockedOn;
	OwnerCharacter->GetCharacterMovement()->bOrientRotationToMovement = !bLockedOn;
	CastChecked<UMaidAnimInstance>(OwnerCharacter->GetMesh()->GetAnimInstance())->bIsLockedOn = bLockedOn;

	SetComponentTickEnabled(bLockedOn);
}

void ULockOnComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                     FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// the target died or was destroyed
	// attempt to lock on the next best one, or drop the lock
	if (!IsTargetable(CurrentTarget))
	{
		TryLockOn();
	}
}
