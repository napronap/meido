#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SandboxGameMode.generated.h"

class AEnemyMaid;

// Straight into gameplay, to iterate on maps and AI: no menu, flow states, flow cameras or pause
// Rules: every wave spawner starts right away, enemy AI is always on unless frozen by K
UCLASS()
class MEIDO_5_6_API ASandboxGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	virtual void StartPlay() override;

	// K (ASandboxPlayerController)
	void ToggleDebugEnemyAI();

private:
	void SetAllEnemyAIEnabled() const;

	UFUNCTION()
	void HandleEnemySpawned(AEnemyMaid* SpawnedEnemy);

	bool bDebugEnemyAIDisabled = false;
};
