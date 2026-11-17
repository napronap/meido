// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/EnemyMaid.h"
#include "ActorComponents/HealthComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "IA/EnemyMaidAIController.h"

AEnemyMaid::AEnemyMaid()
{
	AIControllerClass = AEnemyMaidAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	bUseControllerRotationYaw = true;

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->bOrientRotationToMovement = false;
		MovementComponent->bUseRVOAvoidance = true;
		MovementComponent->AvoidanceConsiderationRadius = 180.f;
	}
}

FVector AEnemyMaid::GetTargetLocation_Implementation()
{
	if (USkeletalMeshComponent* MeshComponent = GetMesh())
	{
		return MeshComponent->Bounds.Origin;
	}

	return GetActorLocation();
}

bool AEnemyMaid::CanBeTargeted_Implementation() const
{
	if (HealthComponent && HealthComponent->IsDead())
	{
		return false;
	}

	return true;
}

