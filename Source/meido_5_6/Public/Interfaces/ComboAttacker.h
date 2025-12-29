// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ComboAttacker.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable)
class UComboAttacker : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class MEIDO_5_6_API IComboAttacker
{
	GENERATED_BODY()

public:
	// performs the actual check for the combo, called from the anim notify
	// still not sure if I really need to make it a BlueprintNativeEvent, since I'm not using this interface in BP
	// but adding it like this registers this function to the reflection system
	UFUNCTION(BlueprintNativeEvent, Category="Attacker")
	void CheckCombo();
};
