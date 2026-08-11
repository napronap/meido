// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Types/FlowTypes.h"
#include "MeidoDemoGameMode.generated.h"

class ADemoMaidPlayerController;
class AEnemyWaveSpawner;
class AEnemyMaid;
class UHealthComponent;
class APawn;
class AActor;
class AMaidCharacter;

/**
 * Demo product GameMode (CP4.1): owns flow FSM + win/lose rules.
 * Presentation (widgets, input mode, camera transitions) stays on DemoMaidPlayerController.
 * Assign this GM on the demo map World Settings.
 */
UCLASS()
class MEIDO_5_6_API AMeidoDemoGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMeidoDemoGameMode();

	UFUNCTION(BlueprintPure, Category = "Flow")
	EFlowState GetFlowState() const { return FlowState; }

	UFUNCTION(BlueprintPure, Category = "Flow")
	bool IsWaitingForMenuCamera() const { return bWaitingForMenuCameraTransition; }

	UFUNCTION(BlueprintPure, Category = "Flow")
	bool IsWaitingForWinSequence() const { return bWaitingForWinSequence; }

	UFUNCTION(BlueprintPure, Category = "Flow")
	bool IsWaitingForLoseSequence() const { return bWaitingForLoseSequence; }

	UPROPERTY(BlueprintAssignable, Category = "Flow")
	FOnFlowStateChanged OnFlowStateChanged;

	/** Widget / PC: start or retry from menu / win / lose. */
	UFUNCTION(BlueprintCallable, Category = "Flow")
	void RequestStartFromMenu();

	/** Demo character camera finished menu → gameplay. */
	UFUNCTION(BlueprintCallable, Category = "Flow")
	void NotifyMenuCameraFinished();

	UFUNCTION(BlueprintCallable, Category = "Flow")
	void NotifyWinCameraFinished();

	UFUNCTION(BlueprintCallable, Category = "Flow")
	void NotifyLoseCameraFinished();

	UFUNCTION(BlueprintCallable, Category = "Flow")
	void RequestTogglePause();

	UFUNCTION(BlueprintCallable, Category = "Flow")
	void RequestResumeFromPause();

	UFUNCTION(BlueprintCallable, Category = "Flow")
	void RequestQuit();

	/** Restart path (same as start-from-result). */
	UFUNCTION(BlueprintCallable, Category = "Flow")
	void RequestRestartRun();

	/** PC OnPossess: rebind player health for lose path. */
	void NotifyPlayerPawnReady(APawn* Pawn);

	AEnemyWaveSpawner* GetBoundWaveSpawner() const { return BoundWaveSpawner; }

protected:
	virtual void BeginPlay() override;

	/** If true, find first AEnemyWaveSpawner in world on BeginPlay. */
	UPROPERTY(EditAnywhere, Category = "Flow|Spawner")
	bool bAutoFindWaveSpawner = true;

private:
	UPROPERTY(VisibleAnywhere, Category = "Flow")
	EFlowState FlowState = EFlowState::MainMenu;

	UPROPERTY(Transient)
	TObjectPtr<AEnemyWaveSpawner> BoundWaveSpawner = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UHealthComponent> BoundPlayerHealth = nullptr;

	bool bWaitingForMenuCameraTransition = false;
	bool bWaitingForWinSequence = false;
	bool bWaitingForLoseSequence = false;

	void SetFlowState(EFlowState NewState);
	void BindFlowSources();
	void BindPlayerHealthFromPawn(APawn* Pawn);
	void EnterMainMenu();
	void EnterPlaying();
	void EnterPaused();
	void EnterWin();
	void EnterLose();
	void StartWinSequence();
	void StartLoseSequence();
	void ClearEnemiesAfterLose();
	void PrepareRestartFromResult();

	ADemoMaidPlayerController* GetDemoPC() const;
	APawn* GetPlayerPawn() const;
	AMaidCharacter* GetPlayerMaid() const;

	UFUNCTION()
	void HandleWaveCompleted();

	UFUNCTION()
	void HandleEnemySpawned(AEnemyMaid* SpawnedEnemy);

	UFUNCTION()
	void HandlePlayerHealthDepleted(UHealthComponent* HealthComponent, AActor* DamageCauser);
};
