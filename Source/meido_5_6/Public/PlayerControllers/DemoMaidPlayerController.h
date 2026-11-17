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
class UMeidoTutorialWidget;
class UMeidoHUD;
class UDemoFlowAudioData;
class USoundBase;

/** Which menu list is active for pad nav (CP4.2). */
UENUM(BlueprintType)
enum class EDemoMenuKind : uint8
{
	None UMETA(DisplayName = "None"),
	MainMenu UMETA(DisplayName = "MainMenu"),
	Pause UMETA(DisplayName = "Pause"),
	Win UMETA(DisplayName = "Win"),
	Lose UMETA(DisplayName = "Lose"),
	Tutorial UMETA(DisplayName = "Tutorial"),
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnDemoMenuSelectionChanged,
	EDemoMenuKind, MenuKind,
	int32, SelectedIndex
);

/**
 * Demo PC: presentation + IMC swap + simple menu index nav (CP4.1–4.2).
 * Flow rules: AMeidoDemoGameMode.
 */
UCLASS()
class MEIDO_5_6_API ADemoMaidPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	/** Playing: move, look, combat, pause. */
	UPROPERTY(EditAnywhere, Category = "Input|IMC")
	TObjectPtr<UInputMappingContext> GameplayIMC;

	/** Legacy name alias — if set and GameplayIMC null, used as gameplay. */
	UPROPERTY(EditAnywhere, Category = "Input|IMC")
	TObjectPtr<UInputMappingContext> DefaultIMC;

	/** Menu/Pause/Win/Lose: NavigateUp/Down, Confirm, Cancel. */
	UPROPERTY(EditAnywhere, Category = "Input|IMC")
	TObjectPtr<UInputMappingContext> MenuIMC;

	UPROPERTY(EditAnywhere, Category = "Input|Actions")
	TObjectPtr<UInputAction> PauseInputAction;

	UPROPERTY(EditAnywhere, Category = "Input|Actions")
	TObjectPtr<UInputAction> MenuNavigateUpAction;

	UPROPERTY(EditAnywhere, Category = "Input|Actions")
	TObjectPtr<UInputAction> MenuNavigateDownAction;

	UPROPERTY(EditAnywhere, Category = "Input|Actions")
	TObjectPtr<UInputAction> MenuConfirmAction;

	/** Pause: resume. Other menus: ignored for now. */
	UPROPERTY(EditAnywhere, Category = "Input|Actions")
	TObjectPtr<UInputAction> MenuCancelAction;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> MainMenuWidgetClass;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> PauseMenuWidgetClass;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> WinWidgetClass;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> LoseWidgetClass;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UMeidoTutorialWidget> TutorialWidgetClass;

	UPROPERTY(EditAnywhere, Category = "UI|HUD")
	TSubclassOf<UMeidoHUD> GameplayHUDClass;

	UPROPERTY(EditAnywhere, Category = "UI|Audio")
	TObjectPtr<UDemoFlowAudioData> FlowAudio;

	UPROPERTY(EditAnywhere, Category = "UI|Transition", meta = (ClampMin = "0.0"))
	float RetryFadeDuration = 0.2f;

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnUnPossess() override;

public:
	bool CanPresentTutorial() const;

	UFUNCTION(BlueprintCallable, Category = "Flow|Tutorial")
	void AdvanceTutorial();

	UFUNCTION(BlueprintCallable, Category = "Flow|Tutorial")
	void PreviousTutorialSlide();

	/** Mouse hover/click use the same selection and actions as the gamepad. */
	UFUNCTION(BlueprintCallable, Category = "Flow|Menu")
	void SelectMenuEntry(int32 InMenuIndex);

	UFUNCTION(BlueprintCallable, Category = "Flow|Menu")
	void ActivateMenuEntry(int32 InMenuIndex);

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

	UPROPERTY(BlueprintAssignable, Category = "Flow")
	FOnFlowStateChanged OnFlowStateChanged;

	/** WBP highlight: which row is selected (0-based). */
	UPROPERTY(BlueprintAssignable, Category = "Flow|Menu")
	FOnDemoMenuSelectionChanged OnMenuSelectionChanged;

	UFUNCTION(BlueprintPure, Category = "Flow|Menu")
	EDemoMenuKind GetActiveMenuKind() const { return ActiveMenuKind; }

	UFUNCTION(BlueprintPure, Category = "Flow|Menu")
	int32 GetMenuSelectedIndex() const { return MenuSelectedIndex; }

	UFUNCTION(BlueprintPure, Category = "Flow|Menu")
	int32 GetMenuOptionCount() const;

	void ApplyFlowPresentation(EFlowState NewState);
	void PrepareLocalRestartPresentation();
	void ApplyRestartInterimPresentation();
	void ApplyPreWinPresentation();
	void ApplyPreLosePresentation();

	bool IsDebugEnemyAIDisabled() const { return bDebugDisableEnemyAI; }
	void SetEnemyAIEnabledPublic(AEnemyMaid* Enemy, bool bEnabled);

private:
	UPROPERTY(Transient)
	TObjectPtr<UMeidoTutorialWidget> TutorialWidget;

	UPROPERTY(Transient)
	TObjectPtr<UMeidoHUD> GameplayHUD;

	bool bHasPresentedFlow = false;
	EFlowState LastPresentedFlow = EFlowState::MainMenu;

	bool CanAcceptMenuInput() const;
	void ShowTutorialUI();
	void SetGameplayHUDVisible(bool bInVisible);
	void PlayFlowSound(USoundBase* InFlowSound) const;
	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> MainMenuWidget = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> PauseMenuWidget = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> WinWidget = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> LoseWidget = nullptr;

	bool bDebugDisableEnemyAI = false;

	EDemoMenuKind ActiveMenuKind = EDemoMenuKind::None;
	int32 MenuSelectedIndex = 0;

	void OnPauseInput(const FInputActionValue& Value);
	void OnMenuNavigateUp(const FInputActionValue& Value);
	void OnMenuNavigateDown(const FInputActionValue& Value);
	void OnMenuConfirm(const FInputActionValue& Value);
	void OnMenuCancel(const FInputActionValue& Value);
	void OnDebugToggleEnemyAI();

	void ShowMainMenuUI();
	void ShowPlayingUI();
	void ShowPausedUI();
	void ShowWinUI();
	void ShowLoseUI();

	void ApplyInputMappingForFlow(EFlowState State);
	UInputMappingContext* ResolveGameplayIMC() const;
	void SetMenuContext(EDemoMenuKind Kind);
	void MoveMenuSelection(int32 Delta);
	void ConfirmMenuSelection();
	void BroadcastMenuSelection();

	UUserWidget* EnsureWidget(TObjectPtr<UUserWidget>& WidgetInstance, TSubclassOf<UUserWidget> WidgetClass);
	void SetGameplayInputEnabled(bool bEnabled);
	void SetAllEnemyAIEnabled(bool bEnabled);
	void SetEnemyAIEnabled(AEnemyMaid* Enemy, bool bEnabled);
	void HideAllFlowWidgets();

	AMeidoDemoGameMode* GetDemoGameMode() const;

	UFUNCTION()
	void HandleGameModeFlowChanged(EFlowState NewState);
};
