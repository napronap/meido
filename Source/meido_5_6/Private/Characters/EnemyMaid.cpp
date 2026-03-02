// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/EnemyMaid.h"
#include "GameFramework/CharacterMovementComponent.h"

AEnemyMaid::AEnemyMaid()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AEnemyMaid::BeginPlay()
{
	Super::BeginPlay();

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->StopMovementImmediately();
		MovementComponent->DisableMovement();
	}
}

