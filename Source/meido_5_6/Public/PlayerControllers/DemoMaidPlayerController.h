// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "Types/FlowTypes.h"
#include "DemoMaidPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class UUserWidget;
class AEnemyMaid;
class AMeidoDemoGameMode;

/**
 * Demo PC (CP4.1): local presentation + input.
 * Flow state machine lives on AMeidoDemoGameMode — this class applies widgets / input mode / AI flags.
 */
UCLASS()
class MEIDO_5_6_API ADemoMaidPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = Input)
	UInputMappingContext* DefaultIMC;

	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* PauseInputAction;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> MainMenuWidgetClass;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> PauseMenuWidgetClass;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> WinWidgetClass;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> LoseWidgetClass;

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override;

public:
	// --- BP / widgets: forward to GameMode ---

	UFUNCTION(BlueprintCallable, Category = "Flow")
	void StartGameFromMenu();

	UFUNCTION(BlueprintCallable, Category = "Flow")
	void CompleteStartGameFromMenu();

	UFUNCTION(BlueprintCallable, Category = "Flow")
	void CompleteWinSequence();

	UFUNCTION(BlueprintCallable, Category = "Flow")
	void CompleteLoseSequence();

	UFUNCTION(BlueprintCallable, Category = "Flow")
	void ResumeGameFromPause();

	UFUNCTION(BlueprintCallable, Category = "Flow")
	void QuitGameRequested();

	UFUNCTION(BlueprintCallable, Category = "Flow")
	void RestartCurrentLevel();

	UFUNCTION(BlueprintCallable, Category = "Flow")
	void TogglePauseMenu();

	UFUNCTION(BlueprintPure, Category = "Flow")
	EFlowState GetFlowState() const;

	/** Still broadcast for existing BP listeners; mirrors GM. */
	UPROPERTY(BlueprintAssignable, Category = "Flow")
	FOnFlowStateChanged OnFlowStateChanged;

	// --- Called by AMeidoDemoGameMode ---

	void ApplyFlowPresentation(EFlowState NewState);
	void PrepareLocalRestartPresentation();
	void ApplyRestartInterimPresentation();
	void ApplyPreWinPresentation();
	void ApplyPreLosePresentation();

	bool IsDebugEnemyAIDisabled() const { return bDebugDisableEnemyAI; }
	void SetEnemyAIEnabledPublic(AEnemyMaid* Enemy, bool bEnabled);

private:
	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> MainMenuWidget = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> PauseMenuWidget = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> WinWidget = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> LoseWidget = nullptr;

	bool bDebugDisableEnemyAI = false;

	void OnPauseInput(const FInputActionValue& Value);
	void OnDebugToggleEnemyAI();

	void ShowMainMenuUI();
	void ShowPlayingUI();
	void ShowPausedUI();
	void ShowWinUI();
	void ShowLoseUI();

	UUserWidget* EnsureWidget(TObjectPtr<UUserWidget>& WidgetInstance, TSubclassOf<UUserWidget> WidgetClass);
	void SetGameplayInputEnabled(bool bEnabled);
	void SetAllEnemyAIEnabled(bool bEnabled);
	void SetEnemyAIEnabled(AEnemyMaid* Enemy, bool bEnabled);
	void HideAllFlowWidgets();

	AMeidoDemoGameMode* GetDemoGameMode() const;

	UFUNCTION()
	void HandleGameModeFlowChanged(EFlowState NewState);
};
