// Fill out your copyright notice in the Description page of Project Settings.


#include "MeiDouResults/OmuriceBomb.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystems/CombatFeedbackSubsystem.h"

AOmuriceBomb::AOmuriceBomb()
{
	PrimaryActorTick.bCanEverTick = false;

	// only static geometry stops it and goes through characters
	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	Collision->InitSphereRadius(20.f);
	Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Collision->SetCollisionObjectType(ECC_WorldDynamic);
	Collision->SetCollisionResponseToAllChannels(ECR_Ignore);
	Collision->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);

	SetRootComponent(Collision);

	BombMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BombMesh"));
	BombMesh->SetupAttachment(Collision);
	// very important !! so that the omelette goes past everyone
	BombMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BombMesh->SetGenerateOverlapEvents(false);

	Movement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Movement"));
	Movement->InitialSpeed = 0.f;
	Movement->bShouldBounce = false;
	// ~1 s from InitialHeightOffset (1800) with this scale
	Movement->ProjectileGravityScale = 3.5f;
}

void AOmuriceBomb::BeginPlay()
{
	Super::BeginPlay();

	// MeiDouComponent already sets the target so the location here is 0,0 and the height
	AddActorWorldOffset(FVector(0.f, 0.f, InitialHeightOffset));

	Movement->OnProjectileStop.AddDynamic(this, &AOmuriceBomb::HandleImpact);
}

void AOmuriceBomb::HandleImpact(const FHitResult& ImpactResult)
{
	UWorld* World = GetWorld();
	const FVector ImpactLocation = GetActorLocation();
	APawn* Caster = GetInstigator();
	
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(OmuriceBombImpact), false, this);
	QueryParams.AddIgnoredActor(Caster);

	TArray<FOverlapResult> Overlaps;
	// same comment as the AttackComponent, it's by object Pawn but should consider otherwise
	World->OverlapMultiByObjectType(
		Overlaps,
		ImpactLocation,
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(ImpactRadius),
		QueryParams
	);

	UCombatFeedbackSubsystem* CombatFeedback = World->GetSubsystem<UCombatFeedbackSubsystem>();
	TSet<AActor*> DamagedActors;
	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* HitActor = Overlap.GetActor();
		if (DamagedActors.Contains(HitActor))
		{
			continue;
		}

		const float AppliedDamage = UGameplayStatics::ApplyDamage(
			HitActor,
			ImpactDamage,
			GetInstigatorController(),
			this,
			nullptr
		);

		if (AppliedDamage > 0.f)
		{
			DamagedActors.Add(HitActor);
			CombatFeedback->ApplyHitStop(HitActor, ImpactHitStopDuration);
		}
	}

	if (DamagedActors.Num() > 0)
	{
		CombatFeedback->ApplyHitStop(Caster, ImpactHitStopDuration);
	}
	
	DrawDebugSphere(World, ImpactLocation, ImpactRadius, 24, FColor::Orange, false, 1.5f);

	SetActorEnableCollision(false);
	SetLifeSpan(DestroyDelay);
}
