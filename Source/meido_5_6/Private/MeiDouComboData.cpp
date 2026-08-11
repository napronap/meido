// Fill out your copyright notice in the Description page of Project Settings.

#include "MeiDouComboData.h"

const FMeiDouComboDefinition* UMeiDouComboData::FindDefinition(const TArray<EMeiDouInput>& Sequence) const
{
	for (const FMeiDouRecipe& Recipe : Recipes)
	{
		if (Recipe.Matches(Sequence))
		{
			return &Recipe.Definition;
		}
	}

	// Legacy TMap path (existing Content DA).
	if (Sequence.Num() == 3)
	{
		FMeiDouComboKey Key;
		Key.A = Sequence[0];
		Key.B = Sequence[1];
		Key.C = Sequence[2];
		if (const FMeiDouComboDefinition* Def = Combos.Find(Key))
		{
			return Def;
		}
	}

	return nullptr;
}

void UMeiDouComboData::PreloadSpawnClasses() const
{
	auto PreloadDef = [](const FMeiDouComboDefinition& Def)
	{
		if (Def.ResultType != EMeiDouResultType::EMDRT_Spawn)
		{
			return;
		}
		Def.SpawnConfig.PreloadSpawnClass();
	};

	for (const FMeiDouRecipe& Recipe : Recipes)
	{
		PreloadDef(Recipe.Definition);
	}

	for (const TPair<FMeiDouComboKey, FMeiDouComboDefinition>& Pair : Combos)
	{
		PreloadDef(Pair.Value);
	}
}
