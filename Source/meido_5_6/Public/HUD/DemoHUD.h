#pragma once

#include "CoreMinimal.h"
#include "HUD/MaidHUD.h"
#include "GameModes/DemoGameMode.h"
#include "DemoHUD.generated.h"

class UUserWidget;

// The gameplay HUD (AMaidHUD)
// (ADemoGameMode::OnFlowStateChanged) with its input mode + cursor
UCLASS()
class MEIDO_5_6_API ADemoHUD : public AMaidHUD
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void HandleFlowStateChanged(EFlowState NewState);
	
	UPROPERTY(EditDefaultsOnly, Category="Flow")
	TMap<EFlowState, TSubclassOf<UUserWidget>> FlowWidgets;

	UPROPERTY()
	TObjectPtr<UUserWidget> FlowWidget;
};
