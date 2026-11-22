// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/MaidCharacter.h"
#include "Interfaces/Targetable.h"
#include "EnemyMaid.generated.h"

class UWidgetComponent;

/**
 *
 */
UCLASS()
class MEIDO_5_6_API AEnemyMaid : public AMaidCharacter, public ITargetable
{
	GENERATED_BODY()

public:
	AEnemyMaid();

	// where the marker is drawn, which is also where the lockon camera aims
	virtual FVector GetTargetLocation_Implementation() override;
	virtual bool CanBeTargeted_Implementation() const override;
	virtual void SetLockedOn_Implementation(bool bLockedOn) override;

protected:
	// needs to be screen space!!
	UPROPERTY(VisibleAnywhere, Category="LockOn")
	TObjectPtr<UWidgetComponent> LockOnMarker;
};
