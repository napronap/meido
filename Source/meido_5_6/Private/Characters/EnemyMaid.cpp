// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/EnemyMaid.h"
#include "ActorComponents/HealthComponent.h"
#include "Components/SkeletalMeshComponent.h"

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

