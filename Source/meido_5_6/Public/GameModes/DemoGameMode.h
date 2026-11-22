#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameModes/DemoCameraManager.h"
#include "DemoGameMode.generated.h"

class AEnemyMaid;
class AEnemyWaveSpawner;
class APlayerController;
class APlayerMaidCharacter;
class UAnimMontage;
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
//  Win/Lose/Paused --StartGame (retry)--> Starting
//
// player input only while Playing, enemy AI while Playing or Paused (and not frozen by K)
// The transitions last as long as their camera blend (ADemoCameraManager, set by ADemoPlayerController)
// The maid's pose and expression per state are set here: menu pose + wink, neutral once Playing, win montage + happy
// Listeners: ADemoHUD (flow widgets, gameplay HUD visibility)
UCLASS()
class MEIDO_5_6_API ADemoGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	// after Super every actor already ran BeginPlay, guarantees the player is possessed and the HUD is listening
	virtual void StartPlay() override;

	// main menu Start and the Win/Lose/Pause retry buttons
	UFUNCTION(BlueprintCallable, Category="Flow")
	void StartGame();

	// P and K (ADemoPlayerController)
	void TogglePause();
	void ToggleDebugEnemyAI();

	UFUNCTION(BlueprintPure, Category="Flow")
	EFlowState GetFlowState() const { return FlowState; }

	UPROPERTY(BlueprintAssignable, Category="Flow")
	FOnFlowStateChanged OnFlowStateChanged;

private:
	void SetFlowState(EFlowState NewState);

	// enters Starting / Winning / Losing, which last as long as the camera blend
	void StartTransition(EFlowState TransitionState, float BlendTime);

	// Starting -> Playing, Winning -> Win, Losing -> Lose
	void FinishTransition();

	// stops the spawner and destroys the enemies still alive (lose screen, retry)
	void ClearWave() const;

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

	// camera per flow state: menu (snapped at start) and win in front of the maid, lose from above
	UPROPERTY(EditDefaultsOnly, Category="Camera")
	FFlowCameraShot MenuShot{.ArmLength = 130.f, .Pitch = -30.f, .YawOffset = 150.f, .SideOffset = -40.f};

	UPROPERTY(EditDefaultsOnly, Category="Camera")
	FFlowCameraShot WinShot{.ArmLength = 130.f, .Pitch = -30.f, .YawOffset = 150.f, .SideOffset = -40.f};

	UPROPERTY(EditDefaultsOnly, Category="Camera")
	FFlowCameraShot LoseShot{.ArmLength = 420.f, .Pitch = -75.f, .BlendTime = 1.f};

	// Starting: back to the maid's own camera (its spring arm keeps its length and offset)
	UPROPERTY(EditDefaultsOnly, Category="Camera")
	float GameplayPitch = -10.f;

	UPROPERTY(EditDefaultsOnly, Category="Camera", meta=(ClampMin="0.01"))
	float GameplayBlendTime = 1.25f;

	// main menu: the maid holds this montage's pose, played from MenuPoseSection
	UPROPERTY(EditDefaultsOnly, Category="Flow")
	TObjectPtr<UAnimMontage> MenuPoseMontage;

	UPROPERTY(EditDefaultsOnly, Category="Flow")
	FName MenuPoseSection = TEXT("PoseStart");

	UPROPERTY(EditDefaultsOnly, Category="Flow")
	TObjectPtr<UAnimMontage> WinMontage;

	FTimerHandle TransitionTimerHandle;

	UPROPERTY()
	TObjectPtr<APlayerController> PlayerController;

	UPROPERTY()
	TObjectPtr<ADemoCameraManager> CameraManager;

	UPROPERTY()
	TObjectPtr<APlayerMaidCharacter> PlayerMaid;

	UPROPERTY()
	TObjectPtr<AEnemyWaveSpawner> WaveSpawner;
};
