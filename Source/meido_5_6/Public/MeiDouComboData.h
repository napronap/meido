// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Types/MeiDouTypes.h"
#include "MeiDouComboData.generated.h"

/**
 * Data asset that represents combos resulting of using MeiDou
 */
UCLASS()
class MEIDO_5_6_API UMeiDouComboData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<FMeiDouComboKey, FMeiDouComboDefinition> Combos;
};
