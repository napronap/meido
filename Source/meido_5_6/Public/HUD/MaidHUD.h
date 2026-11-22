#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "MaidHUD.generated.h"

class UHealthComponent;
class UPlayerHUDWidget;

// Gameplay HUD for any game mode: the player's health bar (PlayerHUDClass, WBP_PlayerHUD)
// Each game mode picks its HUD class; ADemoHUD adds the flow widgets on top
UCLASS()
class MEIDO_5_6_API AMaidHUD : public AHUD
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

	void SetGameplayHUDVisible(bool bVisible) const;

private:
	UFUNCTION()
	void HandleHealthChanged(UHealthComponent* HealthComponent, float CurrentHealth, float MaxHealth, float Delta);

	UPROPERTY(EditDefaultsOnly, Category="HUD")
	TSubclassOf<UPlayerHUDWidget> PlayerHUDClass;

	UPROPERTY()
	TObjectPtr<UPlayerHUDWidget> PlayerHUD;
};
