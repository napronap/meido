// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorComponents/AttackComponent.h"
#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Engine/World.h"

// Sets default values for this component's properties
UAttackComponent::UAttackComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UAttackComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ACharacter>(GetOwner());
}

void UAttackComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	for (auto& Pair : ActiveHitStopTimers)
	{
		if (AActor* TargetActor = Pair.Key.Get())
		{
			RestoreLocalTimeDilation(TargetActor);
		}
	}

	ActiveHitStopTimers.Reset();

	Super::EndPlay(EndPlayReason);
}

void UAttackComponent::StartAttack(EHitStopType InHitStopType)
{
	if (bIsAttacking) return;

	bIsAttacking = true;
	HitActorsThisAttack.Reset();
	CurrentHitStopType = InHitStopType;
}

void UAttackComponent::OpenHitWindow(FName InSocket)
{
	CurrentHitSocket = InSocket;
	// Allow each hit window (each swing/section) to damage targets once.
	HitActorsThisAttack.Reset();
	bHitWindowOpen = true;
}

void UAttackComponent::CloseHitWindow()
{
	bHitWindowOpen = false;
	bIsAttacking = false;
}

void UAttackComponent::PerformHitTrace()
{
	if (!OwnerCharacter) return;

	const FVector Start = OwnerCharacter->GetMesh()->GetSocketLocation(CurrentHitSocket);

	const FVector End = Start + OwnerCharacter->GetActorForwardVector() * TraceDistance;

	FCollisionShape Box = FCollisionShape::MakeBox(BoxExtent);
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(AttackTrace), false, OwnerCharacter);
	QueryParams.AddIgnoredActor(OwnerCharacter);

	FHitResult Hit;

	bool bHit = GetWorld()->SweepSingleByChannel(
		Hit,
		Start,
		End,
		FQuat::Identity,
		ECC_Pawn,
		Box,
		QueryParams
	);

	DrawDebugBox(
		GetWorld(),
		(Start + End) * .5f,
		BoxExtent,
		FColor::Red,
		false,
		.1f
	);

	if (bHit && Hit.GetActor())
	{
		if (!HitActorsThisAttack.Contains(Hit.GetActor()))
		{
			AActor* HitActor = Hit.GetActor();

			UGameplayStatics::ApplyDamage(
				HitActor,
				Damage,
				OwnerCharacter->GetController(),
				OwnerCharacter,
				nullptr
			);

			ApplyHitKnockback(HitActor, CurrentHitStopType);
			ApplyLocalHitStop(OwnerCharacter, CurrentHitStopType);
			ApplyLocalHitStop(HitActor, CurrentHitStopType);

			HitActorsThisAttack.Add(HitActor);
		}
	}
}

void UAttackComponent::ApplyLocalHitStop(AActor* TargetActor, const EHitStopType HitStopType)
{
	if (!TargetActor || !GetWorld())
	{
		return;
	}

	const float Duration = GetHitStopDuration(HitStopType);
	if (Duration <= 0.f)
	{
		return;
	}

	TargetActor->CustomTimeDilation = FMath::Clamp(HitStopTimeDilation, 0.001f, 1.0f);

	FTimerHandle& TimerHandle = ActiveHitStopTimers.FindOrAdd(TargetActor);
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle);

	const TWeakObjectPtr<AActor> WeakActor = TargetActor;
	FTimerDelegate RestoreDelegate;
	RestoreDelegate.BindLambda([this, WeakActor]()
	{
		if (!IsValid(this))
		{
			return;
		}

		if (AActor* ActorToRestore = WeakActor.Get())
		{
			RestoreLocalTimeDilation(ActorToRestore);
		}

		ActiveHitStopTimers.Remove(WeakActor);
	});

	GetWorld()->GetTimerManager().SetTimer(TimerHandle, RestoreDelegate, Duration, false);
}

void UAttackComponent::RestoreLocalTimeDilation(AActor* TargetActor)
{
	if (!TargetActor)
	{
		return;
	}

	TargetActor->CustomTimeDilation = 1.0f;
}

float UAttackComponent::GetHitStopDuration(const EHitStopType HitStopType) const
{
	switch (HitStopType)
	{
	case EHitStopType::Heavy:
		return HeavyHitStopDuration;
	case EHitStopType::Light:
	default:
		return LightHitStopDuration;
	}
}

void UAttackComponent::ApplyHitKnockback(AActor* HitActor, const EHitStopType HitStopType) const
{
	if (!bEnableKnockback || !OwnerCharacter || !HitActor)
	{
		return;
	}

	ACharacter* HitCharacter = Cast<ACharacter>(HitActor);
	if (!HitCharacter)
	{
		return;
	}

	const float KnockbackStrength = GetKnockbackStrength(HitStopType);
	if (KnockbackStrength <= 0.f)
	{
		return;
	}

	if (bRotateTargetTowardAttackerOnHit)
	{
		FVector ToAttacker = OwnerCharacter->GetActorLocation() - HitCharacter->GetActorLocation();
		ToAttacker.Z = 0.f;
		if (!ToAttacker.IsNearlyZero())
		{
			const FRotator FacingAttackerYawOnly(0.f, ToAttacker.Rotation().Yaw, 0.f);
			HitCharacter->SetActorRotation(FacingAttackerYawOnly);
		}
	}

	// Requested behavior: push in the opposite direction of attacker's facing.
	FVector KnockbackDirection = -OwnerCharacter->GetActorForwardVector();
	KnockbackDirection.Z = 0.f;
	KnockbackDirection = KnockbackDirection.GetSafeNormal();

	if (KnockbackDirection.IsNearlyZero())
	{
		// Fallback if forward is invalid for any reason.
		KnockbackDirection = (HitCharacter->GetActorLocation() - OwnerCharacter->GetActorLocation());
		KnockbackDirection.Z = 0.f;
		KnockbackDirection = KnockbackDirection.GetSafeNormal();
	}

	const FVector LaunchVelocity = KnockbackDirection * KnockbackStrength + FVector::UpVector * KnockbackUpwardStrength;
	HitCharacter->LaunchCharacter(LaunchVelocity, true, true);
}

float UAttackComponent::GetKnockbackStrength(const EHitStopType HitStopType) const
{
	switch (HitStopType)
	{
	case EHitStopType::Heavy:
		return HeavyKnockbackStrength;
	case EHitStopType::Light:
	default:
		return LightKnockbackStrength;
	}
}


// Called every frame
void UAttackComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction
)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bHitWindowOpen)
	{
		PerformHitTrace();
	}
}
