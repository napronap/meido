// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/EnemyMaid.h"
#include "ActorComponents/HealthComponent.h"
#include "Components/WidgetComponent.h"
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

	LockOnMarker = CreateDefaultSubobject<UWidgetComponent>(TEXT("LockOnMarker"));
	LockOnMarker->SetupAttachment(GetRootComponent());
	LockOnMarker->SetWidgetSpace(EWidgetSpace::Screen);
	// shown by SetLockedOn
	LockOnMarker->SetVisibility(false);
}

FVector AEnemyMaid::GetTargetLocation_Implementation()
{
	return LockOnMarker->GetComponentLocation();
}

bool AEnemyMaid::CanBeTargeted_Implementation() const
{
	if (HealthComponent && HealthComponent->IsDead())
	{
		return false;
	}

	return true;
}

void AEnemyMaid::SetLockedOn_Implementation(const bool bLockedOn)
{
	// a screen space widget is added to/removed from the screen with its component visibility
	LockOnMarker->SetVisibility(bLockedOn);
}

