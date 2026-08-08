// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/MaidCharacter.h"
#include "Interfaces/CombatTeamSource.h"
#include "Interfaces/Targetable.h"
#include "EnemyMaid.generated.h"

class USimpleHealthBarComponent;

UCLASS()
class MEIDO_5_6_API AEnemyMaid : public AMaidCharacter, public ITargetable, public ICombatTeamSource
{
	GENERATED_BODY()

public:
	AEnemyMaid();

	virtual FVector GetTargetLocation_Implementation() override;
	virtual bool CanBeTargeted_Implementation() const override;
	virtual ECombatTeam GetCombatTeam_Implementation() const override;

protected:
	/** Float HP bar. Widget class + placement: set on this component in BP. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI|Health")
	TObjectPtr<USimpleHealthBarComponent> HealthBarComponent = nullptr;
};
