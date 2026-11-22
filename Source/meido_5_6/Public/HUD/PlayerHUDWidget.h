#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerHUDWidget.generated.h"

// Base of WBP_PlayerHUD: AMaidHUD pushes the player's health into it
UCLASS(Abstract)
class MEIDO_5_6_API UPlayerHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent, Category="HUD")
	void UpdateHealth(float CurrentHealth, float MaxHealth);
};
