// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorComponents/LockOnComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Interfaces/Targetable.h"

// Sets default values for this component's properties
ULockOnComponent::ULockOnComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


// Called when the game starts
void ULockOnComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ACharacter>(GetOwner());
}

void ULockOnComponent::FindTargets(TArray<AActor*>& OutTargets) const
{
	TArray<AActor*> Overlaps;

	UKismetSystemLibrary::SphereOverlapActors(
		GetWorld(),
		OwnerCharacter->GetActorLocation(),
		SearchRadius,
		TargetObjectType,
		nullptr,
		{OwnerCharacter},
		Overlaps
	);

	FString Message1 = FString::Printf(
		TEXT("Overlaps found: %d"), Overlaps.Num());

	GEngine->AddOnScreenDebugMessage(
		-1, 1.5f, FColor::Yellow, Message1
	);

	UE_LOG(LogTemp, Warning, TEXT("Overlaps found: %d"), Overlaps.Num());

	for (AActor* Actor : Overlaps)
	{
		if (!Actor) continue;

		if (Actor->Implements<UTargetable>())
		{
			const bool bCanBeTargeted = ITargetable::Execute_CanBeTargeted(Actor);

			if (bCanBeTargeted)
			{
				OutTargets.Add(Actor);
			}
		}

		FString Message2 = FString::Printf(
			TEXT("Overlap: %s, ImplementsTargetable=%d"), *GetNameSafe(Actor),
			Actor && Actor->Implements<UTargetable>());

		GEngine->AddOnScreenDebugMessage(
			-1, 1.5f, FColor::Yellow, Message2
		);
	}
}

AActor* ULockOnComponent::SelectBestTarget(const TArray<AActor*>& Targets) const
{
	AActor* BestTarget = nullptr;
	float BestDistanceSq = TNumericLimits<float>::Max();

	for (AActor* Target : Targets)
	{
		const float DistSq = FVector::Dist(OwnerCharacter->GetActorLocation(), Target->GetActorLocation());

		if (DistSq < BestDistanceSq)
		{
			BestDistanceSq = DistSq;
			BestTarget = Target;
		}
	}

	return BestTarget;
}

bool ULockOnComponent::TryLockOn()
{
	if (!OwnerCharacter) return false;

	TArray<AActor*> Candidates;
	FindTargets(Candidates);

	if (Candidates.Num() > 0)
	{
		CurrentTarget = SelectBestTarget(Candidates);
		return CurrentTarget != nullptr;
	}

	return false;
}

void ULockOnComponent::ClearLockOn()
{
	CurrentTarget = nullptr;
}

bool ULockOnComponent::IsLockedOn()
{
	return CurrentTarget != nullptr;
}

AActor* ULockOnComponent::GetCurrentTarget()
{
	return CurrentTarget;
}

// Called every frame
void ULockOnComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                     FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}
