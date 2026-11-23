#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "MaidHUD.generated.h"

class UUserWidget;

// Gameplay HUD for any game mode
// shows the player's HUD widget (PlayerHUDClass, WBP_PlayerHUD), which binds itself to the player's health and MeiDou
UCLASS()
class MEIDO_5_6_API AMaidHUD : public AHUD
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

	void SetGameplayHUDVisible(bool bVisible) const;

private:
	UPROPERTY(EditDefaultsOnly, Category="HUD")
	TSubclassOf<UUserWidget> PlayerHUDClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> PlayerHUD;
};
