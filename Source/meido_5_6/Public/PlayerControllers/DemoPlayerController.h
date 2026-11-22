#pragma once

#include "CoreMinimal.h"
#include "PlayerControllers/MaidPlayerController.h"
#include "DemoPlayerController.generated.h"

class UInputAction;

// The demo's player controller: the player's input plus the demo's own (pause, debug AI toggle) for ADemoGameMode,
// and the flow cameras (ADemoCameraManager)
UCLASS()
class MEIDO_5_6_API ADemoPlayerController : public AMaidPlayerController
{
	GENERATED_BODY()

public:
	ADemoPlayerController();

protected:
	virtual void SetupInputComponent() override;

	// needs bTriggerWhenPaused on the asset to unpause
	UPROPERTY(EditDefaultsOnly, Category="Input|Flow")
	TObjectPtr<UInputAction> PauseAction;

	// freezes/unfreezes every enemy
	UPROPERTY(EditDefaultsOnly, Category="Input|Debug")
	TObjectPtr<UInputAction> DebugToggleAIAction;

private:
	void TogglePause();
	void ToggleDebugEnemyAI();
};
