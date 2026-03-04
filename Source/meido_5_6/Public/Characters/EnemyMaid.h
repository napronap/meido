// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/MaidCharacter.h"
#include "Interfaces/Targetable.h"
#include "EnemyMaid.generated.h"

/**
 * 
 */
UCLASS()
class MEIDO_5_6_API AEnemyMaid : public AMaidCharacter, public ITargetable
{
	GENERATED_BODY()

public:
	AEnemyMaid();

	virtual FVector GetTargetLocation_Implementation() override;
	virtual bool CanBeTargeted_Implementation() const override;
};
