#pragma once

#include "CoreMinimal.h"
#include "PlayerControllers/MaidPlayerController.h"
#include "SandboxPlayerController.generated.h"

class UInputAction;

// The sandbox's player controller: the player's input plus the debug AI toggle for ASandboxGameMode
UCLASS()
class MEIDO_5_6_API ASandboxPlayerController : public AMaidPlayerController
{
	GENERATED_BODY()

protected:
	virtual void SetupInputComponent() override;

	// freezes/unfreezes every enemy
	UPROPERTY(EditDefaultsOnly, Category="Input|Debug")
	TObjectPtr<UInputAction> DebugToggleAIAction;

private:
	void ToggleDebugEnemyAI();
};
