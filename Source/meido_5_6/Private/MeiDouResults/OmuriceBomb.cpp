// Fill out your copyright notice in the Description page of Project Settings.


#include "MeiDouResults/OmuriceBomb.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundBase.h"

AOmuriceBomb::AOmuriceBomb()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(SceneRoot);

	BombMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BombMesh"));
	BombMesh->SetupAttachment(SceneRoot);
	BombMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BombMesh->SetCollisionObjectType(ECC_WorldDynamic);
	BombMesh->SetCollisionResponseToAllChannels(ECR_Block);
	BombMesh->SetGenerateOverlapEvents(false);
}

void AOmuriceBomb::BeginPlay()
{
	Super::BeginPlay();

	// Spawn point usually comes from combo target. This offset makes the bomb start from the sky.
	if (InitialHeightOffset > 0.f)
	{
		AddActorWorldOffset(FVector(0.f, 0.f, InitialHeightOffset), false);
	}
}

void AOmuriceBomb::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bHasImpacted || FallSpeed <= 0.f)
	{
		return;
	}

	FHitResult Hit;
	const FVector FallStep(0.f, 0.f, -FallSpeed * DeltaTime);
	AddActorWorldOffset(FallStep, true, &Hit);

	if (Hit.bBlockingHit)
	{
		HandleImpact();
	}
}

void AOmuriceBomb::HandleImpact()
{
	if (bHasImpacted)
	{
		return;
	}

	bHasImpacted = true;

	if (UWorld* World = GetWorld())
	{
		TArray<AActor*> IgnoredActors;
		IgnoredActors.Add(this);

		UGameplayStatics::ApplyRadialDamage(
			World,
			ImpactDamage,
			GetActorLocation(),
			ImpactRadius,
			UDamageType::StaticClass(),
			IgnoredActors,
			this,
			GetInstigatorController(),
			true
		);

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
			DrawDebugSphere(
				World,
				GetActorLocation(),
				ImpactRadius,
				24,
				FColor::Orange,
				false,
				1.5f
			);
		}
	}

	SetActorEnableCollision(false);
	SetActorHiddenInGame(true);
	SetLifeSpan(DestroyDelay);
}
