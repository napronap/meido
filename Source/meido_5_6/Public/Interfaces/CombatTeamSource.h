// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Types/CombatTeamTypes.h"
#include "UObject/Interface.h"
#include "CombatTeamSource.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class UCombatTeamSource : public UInterface
{
	GENERATED_BODY()
};

/**
 * Who owns combat team identity: the pawn/actor class, not AttackComponent.
 * Attack hit filter queries this on owner + target.
 */
class MEIDO_5_6_API ICombatTeamSource
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Combat|Team")
	ECombatTeam GetCombatTeam() const;
};
