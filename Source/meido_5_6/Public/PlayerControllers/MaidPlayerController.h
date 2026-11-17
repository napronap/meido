// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "MaidPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class UUserWidget;
class AEnemyWaveSpawner;
class AEnemyMaid;
class UHealthComponent;
class APawn;
class AActor;

UENUM(BlueprintType)
enum class EFlowState : uint8
{
	MainMenu UMETA(DisplayName = "MainMenu"),
	Playing UMETA(DisplayName = "Playing"),
	Paused UMETA(DisplayName = "Paused"),
	Win UMETA(DisplayName = "Win"),
	Lose UMETA(DisplayName = "Lose"),
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFlowStateChanged, EFlowState, NewState);

/**
 * 
 */
UCLASS()
class MEIDO_5_6_API AMaidPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	// only using one IMC for now
	UPROPERTY(EditAnywhere, Category = Input)
	UInputMappingContext* DefaultIMC;

	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* PauseInputAction;

	UPROPERTY(EditAnywhere, Category="UI")
	TSubclassOf<UUserWidget> MainMenuWidgetClass;

	UPROPERTY(EditAnywhere, Category="UI")
	TSubclassOf<UUserWidget> PauseMenuWidgetClass;

	UPROPERTY(EditAnywhere, Category="UI")
	TSubclassOf<UUserWidget> WinWidgetClass;

	UPROPERTY(EditAnywhere, Category="UI")
	TSubclassOf<UUserWidget> LoseWidgetClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Flow")
	EFlowState CurrentFlowState = EFlowState::MainMenu;
	
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

public:
	UFUNCTION(BlueprintCallable, Category="Flow")
	void StartGameFromMenu();

	UFUNCTION(BlueprintCallable, Category="Flow")
	void CompleteStartGameFromMenu();

	UFUNCTION(BlueprintCallable, Category="Flow")
	void CompleteWinSequence();

	UFUNCTION(BlueprintCallable, Category="Flow")
	void CompleteLoseSequence();

	UFUNCTION(BlueprintCallable, Category="Flow")
	void ResumeGameFromPause();

	UFUNCTION(BlueprintCallable, Category="Flow")
	void QuitGameRequested();

	UFUNCTION(BlueprintCallable, Category="Flow")
	void RestartCurrentLevel();

	UFUNCTION(BlueprintCallable, Category="Flow")
	void TogglePauseMenu();

	UFUNCTION(BlueprintPure, Category="Flow")
	EFlowState GetFlowState() const { return CurrentFlowState; }

	UPROPERTY(BlueprintAssignable, Category="Flow")
	FOnFlowStateChanged OnFlowStateChanged;

private:
	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> MainMenuWidget = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> PauseMenuWidget = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> WinWidget = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> LoseWidget = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<AEnemyWaveSpawner> BoundWaveSpawner = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UHealthComponent> BoundPlayerHealthComponent = nullptr;

	bool bWaitingForMenuCameraTransition = false;
	bool bWaitingForWinSequence = false;
	bool bWaitingForLoseSequence = false;
	bool bDebugDisableEnemyAI = false;

	void OnPauseInput(const FInputActionValue& Value);
	void OnDebugToggleEnemyAI();
	void EnterMainMenuState();
	void EnterPlayingState();
	void EnterPausedState();
	void EnterWinState();
	void EnterLoseState();
	void StartWinSequence();
	void StartLoseSequence();
	void ClearEnemiesAfterLoseTransition();
	void SetFlowState(EFlowState NewState);
	UUserWidget* EnsureWidget(TObjectPtr<UUserWidget>& WidgetInstance, TSubclassOf<UUserWidget> WidgetClass);
	void SetGameplayInputEnabled(bool bEnabled);
	void BindFlowSources();
	void BindPlayerHealth(APawn* InPawn);
	void SetAllEnemyAIEnabled(bool bEnabled);
	void SetEnemyAIEnabled(AEnemyMaid* Enemy, bool bEnabled);

	UFUNCTION()
	void HandleWaveCompleted();

	UFUNCTION()
	void HandleEnemySpawned(AEnemyMaid* SpawnedEnemy);

	UFUNCTION()
	void HandlePlayerHealthDepleted(UHealthComponent* InHealthComponent, AActor* DamageCauser);
};
