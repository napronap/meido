// Fill out your copyright notice in the Description page of Project Settings.

#include "MeiDouResults/OmuriceBomb.h"

#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundBase.h"
#include "Utils/CombatFeedbackSubsystem.h"

AOmuriceBomb::AOmuriceBomb()
{
	PrimaryActorTick.bCanEverTick = true;

	BombMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BombMesh"));
	SetRootComponent(BombMesh);
	// Damage is radial at explode — mesh does not block/fall.
	BombMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BombMesh->SetGenerateOverlapEvents(false);
}

void AOmuriceBomb::BeginPlay()
{
	Super::BeginPlay();

	if (bSnapToGroundOnSpawn)
	{
		SnapToGround();
	}

	ApplyGrowScale(0.f);
	GrowElapsed = 0.f;
	Phase = EPhase::Growing;
	bHasExploded = false;
}

void AOmuriceBomb::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (Phase == EPhase::Growing)
	{
		GrowElapsed += DeltaTime;
		const float SafeDuration = FMath::Max(GrowDuration, KINDA_SMALL_NUMBER);
		const float Alpha = FMath::Clamp(GrowElapsed / SafeDuration, 0.f, 1.f);
		ApplyGrowScale(Alpha);

		if (Alpha >= 1.f)
		{
			Explode();
		}
	}
}

void AOmuriceBomb::Explode()
{
	if (bHasExploded)
	{
		return;
	}

	bHasExploded = true;
	Phase = EPhase::Fading;

	// Lock scale at end.
	ApplyGrowScale(1.f);

	ApplyExplosionDamageAndFeedback();

	if (UWorld* World = GetWorld())
	{
		if (ImpactVFX)
		{
			UGameplayStatics::SpawnEmitterAtLocation(World, ImpactVFX, GetActorTransform());
		}
		if (ImpactSFX)
		{
			UGameplayStatics::PlaySoundAtLocation(World, ImpactSFX, GetActorLocation());
		}
		if (bDrawImpactDebug)
		{
			DrawDebugSphere(World, GetActorLocation(), ImpactRadius, 24, FColor::Orange, false, 1.5f);
		}
	}

	SetActorEnableCollision(false);
	OnStartFade();
}

void AOmuriceBomb::OnStartFade_Implementation()
{
	if (FadeDuration <= 0.f)
	{
		FinishAndDestroy();
		return;
	}

	// Default path: no material work in C++; BP can override for dissolve.
	SetLifeSpan(FadeDuration);
	Phase = EPhase::Done;
	SetActorTickEnabled(false);
}

void AOmuriceBomb::FinishAndDestroy()
{
	if (Phase == EPhase::Done && !IsActorBeingDestroyed())
	{
		// Already scheduled via life span or previous call.
	}

	Phase = EPhase::Done;
	SetActorTickEnabled(false);
	Destroy();
}

void AOmuriceBomb::SnapToGround()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FVector Origin = GetActorLocation();
	const FVector Start = Origin + FVector(0.f, 0.f, GroundSnapUpOffset);
	const FVector End = Origin - FVector(0.f, 0.f, GroundSnapDownDistance);

	FCollisionQueryParams Params(SCENE_QUERY_STAT(OmuriceGroundSnap), false, this);
	Params.AddIgnoredActor(this);
	if (AActor* OwnerActor = GetOwner())
	{
		Params.AddIgnoredActor(OwnerActor);
	}

	FHitResult Hit;
	if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic, Params))
	{
		SetActorLocation(Hit.ImpactPoint + FVector(0.f, 0.f, GroundSnapZOffset), false);
	}
}

void AOmuriceBomb::ApplyGrowScale(const float Alpha)
{
	const float Scale = FMath::Lerp(GrowStartScale, GrowEndScale, Alpha);
	SetActorScale3D(FVector(Scale));
}

void AOmuriceBomb::ApplyExplosionDamageAndFeedback()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	TArray<AActor*> IgnoredActors;
	IgnoredActors.Add(this);
	if (AActor* OwnerActor = GetOwner())
	{
		IgnoredActors.Add(OwnerActor);
	}

	UGameplayStatics::ApplyRadialDamage(
		World,
		ImpactDamage,
		GetActorLocation(),
		ImpactRadius,
		UDamageType::StaticClass(),
		IgnoredActors,
		this,
		GetInstigatorController(),
		true,
		ECC_MAX
	);

	if (!bApplyImpactHitStop)
	{
		return;
	}

	UCombatFeedbackSubsystem* Feedback = World->GetSubsystem<UCombatFeedbackSubsystem>();
	if (!Feedback)
	{
		return;
	}

	if (AActor* OwnerActor = GetOwner(); OwnerActor && bApplyImpactHitStopToOwner)
	{
		Feedback->ApplyHitStop(OwnerActor, ImpactHitStopDuration, ImpactHitStopTimeDilation);
	}

	TArray<FOverlapResult> OverlapResults;
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionQueryParams OverlapQueryParams(SCENE_QUERY_STAT(OmuriceBombHitStopOverlap), false, this);
	OverlapQueryParams.AddIgnoredActor(this);
	if (AActor* OwnerActor = GetOwner())
	{
		OverlapQueryParams.AddIgnoredActor(OwnerActor);
	}

	if (World->OverlapMultiByObjectType(
		OverlapResults,
		GetActorLocation(),
		FQuat::Identity,
		ObjectQueryParams,
		FCollisionShape::MakeSphere(ImpactRadius),
		OverlapQueryParams
	))
	{
		TSet<AActor*> ProcessedActors;
		for (const FOverlapResult& Overlap : OverlapResults)
		{
			AActor* HitActor = Overlap.GetActor();
			if (!HitActor || ProcessedActors.Contains(HitActor))
			{
				continue;
			}

			ProcessedActors.Add(HitActor);
			Feedback->ApplyHitStop(HitActor, ImpactHitStopDuration, ImpactHitStopTimeDilation);
		}
	}
}
