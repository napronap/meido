// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorComponents/AttackComponent.h"
#include "Characters/EnemyMaid.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Characters/PlayerMaidCharacter.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystems/CombatFeedbackSubsystem.h"

UAttackComponent::UAttackComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	// ticks only while a hit window is open
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UAttackComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = CastChecked<ACharacter>(GetOwner());
}

void UAttackComponent::OpenHitWindow(const TArray<FName>& Sockets, const EHitStopType HitStopType)
{
	CurrentHitSockets = Sockets;
	CurrentHitStopType = HitStopType;
	HitActorsThisWindow.Reset();

	// first sweep starts where each socket is when the window opens
	USkeletalMeshComponent* OwnerMesh = OwnerCharacter->GetMesh();
	PreviousSocketLocations.Reset(Sockets.Num());
	for (const FName SocketName : Sockets)
	{
		PreviousSocketLocations.Add(OwnerMesh->GetSocketLocation(SocketName));
	}

	SetComponentTickEnabled(true);
}

void UAttackComponent::CloseHitWindow()
{
	SetComponentTickEnabled(false);
}

void UAttackComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction
)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	PerformHitTrace();
}

void UAttackComponent::PerformHitTrace()
{
	USkeletalMeshComponent* OwnerMesh = OwnerCharacter->GetMesh();
	UWorld* World = GetWorld();

	const FCollisionShape Box = FCollisionShape::MakeBox(BoxExtent);
	const FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(AttackTrace), false, OwnerCharacter);

	for (int32 i = 0; i < CurrentHitSockets.Num(); ++i)
	{
		// sweep the path the box actually traveled since last frame
		// apparently skipping frames is called tunneling
		const FVector Start = PreviousSocketLocations[i];
		const FVector End = OwnerMesh->GetSocketLocation(CurrentHitSockets[i]);
		// previous frame's box will keep this frame's rotation but whatever 
		const FQuat Rotation = OwnerMesh->GetSocketQuaternion(CurrentHitSockets[i]);
		PreviousSocketLocations[i] = End;

		DrawDebugBox(World, Start, BoxExtent, Rotation, FColor::Orange, false, .1f);
		DrawDebugBox(World, End, BoxExtent, Rotation, FColor::Red, false, .1f);

		FHitResult Hit;

		// ObjectType works for now since in this game only pawns hit other pawns
		// if I ever need to hit another thing it may be better to consider creating a channel
		if (!World->SweepSingleByObjectType(Hit, Start, End, Rotation, ECC_Pawn, Box, QueryParams))
		{
			continue;
		}

		AActor* HitActor = Hit.GetActor();
		if (HitActorsThisWindow.Contains(HitActor) || ShouldIgnoreHitActor(HitActor))
		{
			continue;
		}

		HitActorsThisWindow.Add(HitActor);

		const float AppliedDamage = UGameplayStatics::ApplyDamage(
			HitActor,
			Damage,
			OwnerCharacter->GetController(),
			OwnerCharacter,
			nullptr
		);

		// rejected hits (iframes, dead, etc)
		if (AppliedDamage <= 0.f)
		{
			continue;
		}

		const float HitStopDuration = CurrentHitStopType == EHitStopType::Heavy
			? HeavyHitStopDuration
			: LightHitStopDuration;

		UCombatFeedbackSubsystem* CombatFeedback = World->GetSubsystem<UCombatFeedbackSubsystem>();
		CombatFeedback->ApplyHitStop(OwnerCharacter, HitStopDuration);
		CombatFeedback->ApplyHitStop(HitActor, HitStopDuration);
	}
}

bool UAttackComponent::ShouldIgnoreHitActor(const AActor* HitActor) const
{
	if (OwnerCharacter->IsA<AEnemyMaid>())
	{
		return !HitActor->IsA<APlayerMaidCharacter>();
	}

	if (OwnerCharacter->IsA<APlayerMaidCharacter>())
	{
		return !HitActor->IsA<AEnemyMaid>();
	}

	return true;
}
