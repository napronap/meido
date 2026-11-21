#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "DemoGameMode.generated.h"

class AEnemyMaid;
class AEnemyWaveSpawner;
class APlayerController;
class APlayerMaidCharacter;
class UHealthComponent;

UENUM(BlueprintType)
enum class EFlowState : uint8
{
	MainMenu UMETA(DisplayName = "MainMenu"),
	Paused   UMETA(DisplayName = "Paused"),
	
	Starting UMETA(DisplayName = "Starting"),
	Playing  UMETA(DisplayName = "Playing"),
	
	Winning  UMETA(DisplayName = "Winning"),
	Win      UMETA(DisplayName = "Win"),
	
	Losing   UMETA(DisplayName = "Losing"),
	Lose     UMETA(DisplayName = "Lose"),
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFlowStateChanged, EFlowState, NewState);

// Game flow
//
//  MainMenu --StartGame--> Starting --camera done--> Playing <--TogglePause--> Paused
//  Playing --wave completed--> Winning --camera done--> Win
//  Playing --player died--> Losing --camera done--> Lose
//  Win/Lose --StartGame (retry)--> Starting
//
// player input only while Playing, enemy AI while Playing or Paused
// Listeners: ADemoHUD (flow widgets), BP_PlayerMaid (expressions)
UCLASS()
class MEIDO_5_6_API ADemoGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	// after Super every actor already ran BeginPlay, guarantees the player is possessed and the HUD is listening
	virtual void StartPlay() override;

	// main menu Start and the Win/Lose retry buttons
	UFUNCTION(BlueprintCallable, Category="Flow")
	void StartGame();

	void TogglePause();

	// Starting -> Playing, Winning -> Win, Losing -> Lose
	void FinishTransition();
	
	void ToggleDebugEnemyAI();

	UFUNCTION(BlueprintPure, Category="Flow")
	EFlowState GetFlowState() const { return FlowState; }

	UPROPERTY(BlueprintAssignable, Category="Flow")
	FOnFlowStateChanged OnFlowStateChanged;

private:
	void SetFlowState(EFlowState NewState);

	bool IsEnemyAIActive() const;
	void SetAllEnemyAIEnabled() const;

	UFUNCTION()
	void HandleWaveCompleted();

	UFUNCTION()
	void HandleEnemySpawned(AEnemyMaid* SpawnedEnemy);

	UFUNCTION()
	void HandlePlayerHealthDepleted(UHealthComponent* HealthComponent, AActor* DamageCauser);

	UPROPERTY(VisibleInstanceOnly, Category="Flow")
	EFlowState FlowState = EFlowState::MainMenu;

	bool bDebugEnemyAIDisabled = false;

	UPROPERTY()
	TObjectPtr<APlayerController> PlayerController;

	UPROPERTY()
	TObjectPtr<APlayerMaidCharacter> PlayerMaid;

	UPROPERTY()
	TObjectPtr<AEnemyWaveSpawner> WaveSpawner;
};
