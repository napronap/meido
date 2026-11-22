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
	
	GetCharacterMovement()->bUseRVOAvoidance = true;
	GetCharacterMovement()->AvoidanceConsiderationRadius = 180.f;

	// path following moves through acceleration (like player input) instead of setting velocity directly
	// the ABP's ShouldMove needs GetCurrentAcceleration != 0 to enter Walk/Run
	GetCharacterMovement()->GetNavMovementProperties()->bUseAccelerationForPaths = true;
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

