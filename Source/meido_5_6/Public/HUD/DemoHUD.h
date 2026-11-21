#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "GameModes/DemoGameMode.h"
#include "DemoHUD.generated.h"

class UUserWidget;

// Shows the widget of the current flow state (ADemoGameMode::OnFlowStateChanged) and sets input mode + cursor for it
UCLASS()
class MEIDO_5_6_API ADemoHUD : public AHUD
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void HandleFlowStateChanged(EFlowState NewState);

	// states without a widget (Playing and the camera transitions) give the input back to the game
	UPROPERTY(EditDefaultsOnly, Category="Flow")
	TMap<EFlowState, TSubclassOf<UUserWidget>> FlowWidgets;

	UPROPERTY()
	TObjectPtr<UUserWidget> FlowWidget;
};
