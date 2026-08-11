// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Types/MeiDouTypes.h"
#include "MeiDouComboData.generated.h"

/**
 * Catalog of MeiDou techniques (CP3.1+).
 * Prefer Recipes (TArray). Legacy Combos TMap is fallback.
 * CP3.2: PreloadSpawnClasses warms soft result BPs before first cast.
 */
UCLASS()
class MEIDO_5_6_API UMeiDouComboData : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Primary catalog — list in editor, no GetTypeHash required to author. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MeiDou")
	TArray<FMeiDouRecipe> Recipes;

	/**
	 * Legacy map (A/B/C key). Used when Find does not hit Recipes
	 * (or Recipes empty). Prefer migrating entries into Recipes.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MeiDou|Legacy")
	TMap<FMeiDouComboKey, FMeiDouComboDefinition> Combos;

	/** Linear search Recipes, then legacy Combos if Sequence length is 3. */
	const FMeiDouComboDefinition* FindDefinition(const TArray<EMeiDouInput>& Sequence) const;

	/**
	 * Load soft spawn classes for every Spawn recipe (Recipes + legacy Combos).
	 * Call from UMeiDouComponent::PreloadResults — not mid-notify if avoidable.
	 */
	void PreloadSpawnClasses() const;
};
