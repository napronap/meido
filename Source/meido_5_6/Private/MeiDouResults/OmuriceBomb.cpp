// Fill out your copyright notice in the Description page of Project Settings.


#include "MeiDouResults/OmuriceBomb.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
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
	// Movement/impact detection is trace-driven (see Tick), so mesh collision is disabled
	BombMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
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

// TODO: probably rely on gravity? it was easier to do the whole ignoring like this
void AOmuriceBomb::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bHasImpacted || FallSpeed <= 0.f)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FVector Start = GetActorLocation();
	const FVector FallStep(0.f, 0.f, -FallSpeed * DeltaTime);
	const FVector End = Start + FallStep;

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(OmuriceBombFallTrace), false, this);
	QueryParams.AddIgnoredActor(this);
	if (AActor* OwnerActor = GetOwner())
	{
		QueryParams.AddIgnoredActor(OwnerActor);
	}
	
	// bomb ignores everything but ground hit (GroundOnly)
	FHitResult GroundHit;
	const bool bHitGround = World->LineTraceSingleByChannel(
		GroundHit,
		Start,
		End,
		GroundTraceChannel,
		QueryParams
	);

	if (bHitGround && GroundHit.ImpactNormal.Z >= GroundImpactMinNormalZ)
	{
		const FVector GroundOffset = GroundHit.ImpactNormal * 2.f;
		SetActorLocation(GroundHit.ImpactPoint + GroundOffset, false);
		HandleImpact();
		return;
	}

	// No valid impact this frame, keep falling
	SetActorLocation(End, false);
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

		if (bApplyImpactHitStop)
		{
			if (UCombatFeedbackSubsystem* Feedback = World->GetSubsystem<UCombatFeedbackSubsystem>())
			{
				if (AActor* OwnerActor = GetOwner(); OwnerActor && bApplyImpactHitStopToOwner)
				{
					Feedback->ApplyHitStop(
						OwnerActor,
						ImpactHitStopDuration,
						ImpactHitStopTimeDilation
					);
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
						Feedback->ApplyHitStop(
							HitActor,
							ImpactHitStopDuration,
							ImpactHitStopTimeDilation
						);
					}
				}
			}
		}

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
	SetLifeSpan(DestroyDelay);
}
