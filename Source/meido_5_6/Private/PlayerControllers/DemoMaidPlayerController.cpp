// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerControllers/DemoMaidPlayerController.h"

#include "Characters/EnemyMaid.h"
#include "EngineUtils.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameModes/MeidoDemoGameMode.h"
#include "IA/EnemyMaidAIController.h"
#include "InputAction.h"
#include "InputCoreTypes.h"
#include "Blueprint/UserWidget.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Audio/DemoFlowAudioData.h"
#include "Camera/PlayerCameraManager.h"
#include "UI/MeidoHUD.h"
#include "UI/MeidoTutorialWidget.h"
#include "Sound/SoundBase.h"

namespace DemoMenuPrivate
{
	static void EnableActionWhenPaused(UInputAction* Action)
	{
		if (Action)
		{
			Action->bTriggerWhenPaused = true;
		}
	}
}

void ADemoMaidPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (AMeidoDemoGameMode* GM = GetDemoGameMode())
	{
		GM->OnFlowStateChanged.RemoveDynamic(this, &ADemoMaidPlayerController::HandleGameModeFlowChanged);
		GM->OnFlowStateChanged.AddDynamic(this, &ADemoMaidPlayerController::HandleGameModeFlowChanged);
		ApplyFlowPresentation(GM->GetFlowState());
	}
	else
	{
		ShowMainMenuUI();
		SetMenuContext(EDemoMenuKind::MainMenu);
		ApplyInputMappingForFlow(EFlowState::MainMenu);
	}
}

void ADemoMaidPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	DemoMenuPrivate::EnableActionWhenPaused(PauseInputAction);
	DemoMenuPrivate::EnableActionWhenPaused(MenuNavigateUpAction);
	DemoMenuPrivate::EnableActionWhenPaused(MenuNavigateDownAction);
	DemoMenuPrivate::EnableActionWhenPaused(MenuConfirmAction);
	DemoMenuPrivate::EnableActionWhenPaused(MenuCancelAction);

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (PauseInputAction)
		{
			EIC->BindAction(PauseInputAction, ETriggerEvent::Started, this, &ADemoMaidPlayerController::OnPauseInput);
		}
		if (MenuNavigateUpAction)
		{
			EIC->BindAction(MenuNavigateUpAction, ETriggerEvent::Started, this, &ADemoMaidPlayerController::OnMenuNavigateUp);
		}
		if (MenuNavigateDownAction)
		{
			EIC->BindAction(MenuNavigateDownAction, ETriggerEvent::Started, this, &ADemoMaidPlayerController::OnMenuNavigateDown);
		}
		if (MenuConfirmAction)
		{
			EIC->BindAction(MenuConfirmAction, ETriggerEvent::Started, this, &ADemoMaidPlayerController::OnMenuConfirm);
		}
		if (MenuCancelAction)
		{
			EIC->BindAction(MenuCancelAction, ETriggerEvent::Started, this, &ADemoMaidPlayerController::OnMenuCancel);
		}
	}

	if (InputComponent)
	{
		// TODO remove (debug only) — POST 4.b
		InputComponent->BindKey(EKeys::K, IE_Pressed, this, &ADemoMaidPlayerController::OnDebugToggleEnemyAI);
	}
}

void ADemoMaidPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (AMeidoDemoGameMode* GM = GetDemoGameMode())
	{
		GM->NotifyPlayerPawnReady(InPawn);
		ApplyFlowPresentation(GM->GetFlowState());
	}
}

void ADemoMaidPlayerController::OnUnPossess()
{
	if (GameplayHUD)
	{
		GameplayHUD->BindToPawn(nullptr);
	}
	if (AMeidoDemoGameMode* DemoFlow = GetDemoGameMode())
	{
		DemoFlow->NotifyPlayerPawnReady(nullptr);
	}
	Super::OnUnPossess();
}

void ADemoMaidPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (AMeidoDemoGameMode* DemoFlow = GetDemoGameMode())
	{
		DemoFlow->OnFlowStateChanged.RemoveDynamic(this, &ADemoMaidPlayerController::HandleGameModeFlowChanged);
	}
	HideAllFlowWidgets();
	if (GameplayHUD)
	{
		GameplayHUD->RemoveFromParent();
		GameplayHUD = nullptr;
	}
	Super::EndPlay(EndPlayReason);
}

AMeidoDemoGameMode* ADemoMaidPlayerController::GetDemoGameMode() const
{
	return GetWorld() ? GetWorld()->GetAuthGameMode<AMeidoDemoGameMode>() : nullptr;
}

EFlowState ADemoMaidPlayerController::GetFlowState() const
{
	if (const AMeidoDemoGameMode* GM = GetDemoGameMode())
	{
		return GM->GetFlowState();
	}
	return EFlowState::MainMenu;
}

void ADemoMaidPlayerController::HandleGameModeFlowChanged(const EFlowState NewState)
{
	OnFlowStateChanged.Broadcast(NewState);
}

void ADemoMaidPlayerController::StartGameFromMenu()
{
	if (!CanAcceptMenuInput())
	{
		return;
	}
	if (AMeidoDemoGameMode* GM = GetDemoGameMode())
	{
		PlayFlowSound(FlowAudio ? FlowAudio->Confirm.Get() : nullptr);
		GM->RequestStartFromMenu();
	}
}

void ADemoMaidPlayerController::CompleteStartGameFromMenu()
{
	if (AMeidoDemoGameMode* GM = GetDemoGameMode())
	{
		GM->NotifyMenuCameraFinished();
	}
}

void ADemoMaidPlayerController::CompleteWinSequence()
{
	if (AMeidoDemoGameMode* GM = GetDemoGameMode())
	{
		GM->NotifyWinCameraFinished();
	}
}

void ADemoMaidPlayerController::CompleteLoseSequence()
{
	if (AMeidoDemoGameMode* GM = GetDemoGameMode())
	{
		GM->NotifyLoseCameraFinished();
	}
}

void ADemoMaidPlayerController::ResumeGameFromPause()
{
	if (AMeidoDemoGameMode* GM = GetDemoGameMode())
	{
		GM->RequestResumeFromPause();
	}
}

void ADemoMaidPlayerController::QuitGameRequested()
{
	PlayFlowSound(FlowAudio ? FlowAudio->Confirm.Get() : nullptr);
	if (AMeidoDemoGameMode* GM = GetDemoGameMode())
	{
		GM->RequestQuit();
		return;
	}
	UKismetSystemLibrary::QuitGame(this, this, EQuitPreference::Quit, false);
}

void ADemoMaidPlayerController::RestartCurrentLevel()
{
	if (!CanAcceptMenuInput())
	{
		return;
	}
	if (AMeidoDemoGameMode* GM = GetDemoGameMode())
	{
		PlayFlowSound(FlowAudio ? FlowAudio->Confirm.Get() : nullptr);
		GM->RequestRestartRun();
		if (PlayerCameraManager && RetryFadeDuration > 0.f && GM->IsWaitingForMenuCamera())
		{
			PlayerCameraManager->StartCameraFade(1.f, 0.f, RetryFadeDuration, FLinearColor::Black, false, false);
		}
		return;
	}
	StartGameFromMenu();
}

void ADemoMaidPlayerController::TogglePauseMenu()
{
	if (AMeidoDemoGameMode* GM = GetDemoGameMode())
	{
		GM->RequestTogglePause();
	}
}

void ADemoMaidPlayerController::OnPauseInput(const FInputActionValue& Value)
{
	(void)Value;
	TogglePauseMenu();
}

void ADemoMaidPlayerController::OnMenuNavigateUp(const FInputActionValue& Value)
{
	(void)Value;
	if (ActiveMenuKind == EDemoMenuKind::Tutorial)
	{
		PreviousTutorialSlide();
		return;
	}
	MoveMenuSelection(-1);
}

void ADemoMaidPlayerController::OnMenuNavigateDown(const FInputActionValue& Value)
{
	(void)Value;
	if (ActiveMenuKind == EDemoMenuKind::Tutorial)
	{
		AdvanceTutorial();
		return;
	}
	MoveMenuSelection(1);
}

void ADemoMaidPlayerController::OnMenuConfirm(const FInputActionValue& Value)
{
	(void)Value;
	ConfirmMenuSelection();
}

void ADemoMaidPlayerController::OnMenuCancel(const FInputActionValue& Value)
{
	(void)Value;
	if (ActiveMenuKind == EDemoMenuKind::Pause)
	{
		ResumeGameFromPause();
	}
	else if (ActiveMenuKind == EDemoMenuKind::Tutorial)
	{
		PreviousTutorialSlide();
	}
}

void ADemoMaidPlayerController::OnDebugToggleEnemyAI()
{
	bDebugDisableEnemyAI = !bDebugDisableEnemyAI;

	const EFlowState State = GetFlowState();
	const AMeidoDemoGameMode* GM = GetDemoGameMode();
	const bool bWaitingWin = GM && GM->IsWaitingForWinSequence();
	const bool bWaitingLose = GM && GM->IsWaitingForLoseSequence();

	const bool bShouldEnableAI = (State == EFlowState::Playing)
		&& !bWaitingWin
		&& !bWaitingLose
		&& !bDebugDisableEnemyAI;

	SetAllEnemyAIEnabled(bShouldEnableAI);
}

UInputMappingContext* ADemoMaidPlayerController::ResolveGameplayIMC() const
{
	if (GameplayIMC)
	{
		return GameplayIMC.Get();
	}
	return DefaultIMC.Get();
}

void ADemoMaidPlayerController::ApplyInputMappingForFlow(const EFlowState State)
{
	if (!IsLocalPlayerController())
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (!Subsystem)
	{
		return;
	}

	UInputMappingContext* Gameplay = ResolveGameplayIMC();
	const bool bMenu =
		State == EFlowState::MainMenu
		|| State == EFlowState::Tutorial
		|| State == EFlowState::Paused
		|| State == EFlowState::Win
		|| State == EFlowState::Lose;

	if (Gameplay)
	{
		Subsystem->RemoveMappingContext(Gameplay);
	}
	if (MenuIMC)
	{
		Subsystem->RemoveMappingContext(MenuIMC);
	}

	if (bMenu)
	{
		if (MenuIMC)
		{
			Subsystem->AddMappingContext(MenuIMC, 1);
		}
		// Keep pause on gameplay IMC only if also mapped there; prefer Pause on MenuIMC for unpause.
		// Also re-add gameplay at lower priority only if Pause lives there and not on MenuIMC — skip.
	}
	else
	{
		if (Gameplay)
		{
			Subsystem->AddMappingContext(Gameplay, 0);
		}
	}
}

void ADemoMaidPlayerController::SetMenuContext(const EDemoMenuKind Kind)
{
	ActiveMenuKind = Kind;
	MenuSelectedIndex = 0;
	BroadcastMenuSelection();
}

int32 ADemoMaidPlayerController::GetMenuOptionCount() const
{
	switch (ActiveMenuKind)
	{
	case EDemoMenuKind::MainMenu:
		return 2; // Start, Quit
	case EDemoMenuKind::Pause:
		return 3; // Resume, Restart, Quit
	case EDemoMenuKind::Win:
	case EDemoMenuKind::Lose:
		return 2; // Retry, Quit
	case EDemoMenuKind::Tutorial:
		return 1; // Next / Start on the final slide
	default:
		return 0;
	}
}

void ADemoMaidPlayerController::MoveMenuSelection(const int32 Delta)
{
	const int32 Count = GetMenuOptionCount();
	if (!CanAcceptMenuInput() || Count <= 0 || Delta == 0)
	{
		return;
	}

	int32 Next = MenuSelectedIndex + Delta;
	while (Next < 0)
	{
		Next += Count;
	}
	MenuSelectedIndex = Next % Count;
	PlayFlowSound(FlowAudio ? FlowAudio->Navigate.Get() : nullptr);
	BroadcastMenuSelection();
}

void ADemoMaidPlayerController::BroadcastMenuSelection()
{
	OnMenuSelectionChanged.Broadcast(ActiveMenuKind, MenuSelectedIndex);
}

void ADemoMaidPlayerController::ConfirmMenuSelection()
{
	if (!CanAcceptMenuInput())
	{
		return;
	}
	switch (ActiveMenuKind)
	{
	case EDemoMenuKind::Tutorial:
		AdvanceTutorial();
		break;
	case EDemoMenuKind::MainMenu:
		if (MenuSelectedIndex == 0)
		{
			StartGameFromMenu();
		}
		else
		{
			QuitGameRequested();
		}
		break;

	case EDemoMenuKind::Pause:
		if (MenuSelectedIndex == 0)
		{
			ResumeGameFromPause();
		}
		else if (MenuSelectedIndex == 1)
		{
			RestartCurrentLevel();
		}
		else
		{
			QuitGameRequested();
		}
		break;

	case EDemoMenuKind::Win:
	case EDemoMenuKind::Lose:
		if (MenuSelectedIndex == 0)
		{
			RestartCurrentLevel();
		}
		else
		{
			QuitGameRequested();
		}
		break;

	default:
		break;
	}
}

void ADemoMaidPlayerController::ApplyFlowPresentation(const EFlowState NewState)
{
	if (NewState != EFlowState::Tutorial && TutorialWidget)
	{
		TutorialWidget->RemoveFromParent();
	}
	if (bHasPresentedFlow && LastPresentedFlow != NewState && FlowAudio)
	{
		switch (NewState)
		{
		case EFlowState::Paused:
			PlayFlowSound(FlowAudio->Pause.Get());
			break;
		case EFlowState::Playing:
			if (LastPresentedFlow == EFlowState::Paused)
			{
				PlayFlowSound(FlowAudio->Resume.Get());
			}
			break;
		case EFlowState::Win:
			PlayFlowSound(FlowAudio->Win.Get());
			break;
		case EFlowState::Lose:
			PlayFlowSound(FlowAudio->Lose.Get());
			break;
		default:
			break;
		}
	}
	LastPresentedFlow = NewState;
	bHasPresentedFlow = true;
	switch (NewState)
	{
	case EFlowState::MainMenu:
		ShowMainMenuUI();
		SetMenuContext(EDemoMenuKind::MainMenu);
		break;
	case EFlowState::Tutorial:
		ShowTutorialUI();
		SetMenuContext(EDemoMenuKind::Tutorial);
		break;
	case EFlowState::Playing:
		ShowPlayingUI();
		SetMenuContext(EDemoMenuKind::None);
		break;
	case EFlowState::Paused:
		ShowPausedUI();
		SetMenuContext(EDemoMenuKind::Pause);
		break;
	case EFlowState::Win:
		ShowWinUI();
		SetMenuContext(EDemoMenuKind::Win);
		break;
	case EFlowState::Lose:
		ShowLoseUI();
		SetMenuContext(EDemoMenuKind::Lose);
		break;
	default:
		break;
	}

	ApplyInputMappingForFlow(NewState);
	SetGameplayHUDVisible(NewState == EFlowState::Playing || NewState == EFlowState::Paused);
}

void ADemoMaidPlayerController::PrepareLocalRestartPresentation()
{
	HideAllFlowWidgets();
	SetGameplayHUDVisible(false);
	UGameplayStatics::SetGlobalTimeDilation(this, 1.0f);
	SetPause(false);
	SetGameplayInputEnabled(false);
	SetAllEnemyAIEnabled(false);
	SetMenuContext(EDemoMenuKind::None);
}

void ADemoMaidPlayerController::ApplyRestartInterimPresentation()
{
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
	bShowMouseCursor = false;
	SetGameplayInputEnabled(false);
	SetAllEnemyAIEnabled(false);
	ApplyInputMappingForFlow(EFlowState::MainMenu);
}

void ADemoMaidPlayerController::ApplyPreWinPresentation()
{
	HideAllFlowWidgets();
	SetPause(false);
	SetGameplayInputEnabled(false);
	SetAllEnemyAIEnabled(false);
	SetMenuContext(EDemoMenuKind::None);
	ApplyInputMappingForFlow(EFlowState::Win);
	SetGameplayHUDVisible(false);
}

void ADemoMaidPlayerController::ApplyPreLosePresentation()
{
	HideAllFlowWidgets();
	SetPause(false);
	SetGameplayInputEnabled(false);
	SetAllEnemyAIEnabled(false);
	SetMenuContext(EDemoMenuKind::None);
	ApplyInputMappingForFlow(EFlowState::Lose);
	SetGameplayHUDVisible(false);
}

void ADemoMaidPlayerController::HideAllFlowWidgets()
{
	if (TutorialWidget)
	{
		TutorialWidget->RemoveFromParent();
	}
	if (MainMenuWidget && MainMenuWidget->IsInViewport())
	{
		MainMenuWidget->RemoveFromParent();
	}
	if (PauseMenuWidget && PauseMenuWidget->IsInViewport())
	{
		PauseMenuWidget->RemoveFromParent();
	}
	if (WinWidget && WinWidget->IsInViewport())
	{
		WinWidget->RemoveFromParent();
	}
	if (LoseWidget && LoseWidget->IsInViewport())
	{
		LoseWidget->RemoveFromParent();
	}
}

void ADemoMaidPlayerController::ShowMainMenuUI()
{
	if (UUserWidget* Widget = EnsureWidget(MainMenuWidget, MainMenuWidgetClass))
	{
		if (!Widget->IsInViewport())
		{
			Widget->AddToViewport(100);
		}
	}

	if (PauseMenuWidget && PauseMenuWidget->IsInViewport())
	{
		PauseMenuWidget->RemoveFromParent();
	}
	if (WinWidget && WinWidget->IsInViewport())
	{
		WinWidget->RemoveFromParent();
	}
	if (LoseWidget && LoseWidget->IsInViewport())
	{
		LoseWidget->RemoveFromParent();
	}

	FInputModeGameAndUI InputMode;
	if (MainMenuWidget)
	{
		InputMode.SetWidgetToFocus(MainMenuWidget->TakeWidget());
	}
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);

	bShowMouseCursor = true;
	SetPause(false);
	SetGameplayInputEnabled(false);
	SetAllEnemyAIEnabled(false);
}

void ADemoMaidPlayerController::ShowPlayingUI()
{
	HideAllFlowWidgets();

	SetPause(false);
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
	bShowMouseCursor = false;
	SetGameplayInputEnabled(true);
	SetAllEnemyAIEnabled(true);
}

void ADemoMaidPlayerController::ShowPausedUI()
{
	if (UUserWidget* Widget = EnsureWidget(PauseMenuWidget, PauseMenuWidgetClass))
	{
		if (!Widget->IsInViewport())
		{
			Widget->AddToViewport(200);
		}
	}
	if (WinWidget && WinWidget->IsInViewport())
	{
		WinWidget->RemoveFromParent();
	}
	if (LoseWidget && LoseWidget->IsInViewport())
	{
		LoseWidget->RemoveFromParent();
	}

	FInputModeGameAndUI InputMode;
	if (PauseMenuWidget)
	{
		InputMode.SetWidgetToFocus(PauseMenuWidget->TakeWidget());
	}
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);

	bShowMouseCursor = true;
	SetPause(true);
	SetGameplayInputEnabled(false);
}

void ADemoMaidPlayerController::ShowWinUI()
{
	if (MainMenuWidget && MainMenuWidget->IsInViewport())
	{
		MainMenuWidget->RemoveFromParent();
	}
	if (PauseMenuWidget && PauseMenuWidget->IsInViewport())
	{
		PauseMenuWidget->RemoveFromParent();
	}
	if (LoseWidget && LoseWidget->IsInViewport())
	{
		LoseWidget->RemoveFromParent();
	}

	if (UUserWidget* Widget = EnsureWidget(WinWidget, WinWidgetClass))
	{
		if (!Widget->IsInViewport())
		{
			Widget->AddToViewport(300);
		}
	}

	FInputModeGameAndUI InputMode;
	if (WinWidget)
	{
		InputMode.SetWidgetToFocus(WinWidget->TakeWidget());
	}
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);

	bShowMouseCursor = true;
	SetPause(false);
	SetGameplayInputEnabled(false);
	SetAllEnemyAIEnabled(false);
}

void ADemoMaidPlayerController::ShowLoseUI()
{
	if (MainMenuWidget && MainMenuWidget->IsInViewport())
	{
		MainMenuWidget->RemoveFromParent();
	}
	if (PauseMenuWidget && PauseMenuWidget->IsInViewport())
	{
		PauseMenuWidget->RemoveFromParent();
	}
	if (WinWidget && WinWidget->IsInViewport())
	{
		WinWidget->RemoveFromParent();
	}

	if (UUserWidget* Widget = EnsureWidget(LoseWidget, LoseWidgetClass))
	{
		if (!Widget->IsInViewport())
		{
			Widget->AddToViewport(300);
		}
	}

	FInputModeGameAndUI InputMode;
	if (LoseWidget)
	{
		InputMode.SetWidgetToFocus(LoseWidget->TakeWidget());
	}
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);

	bShowMouseCursor = true;
	SetPause(false);
	SetGameplayInputEnabled(false);
	SetAllEnemyAIEnabled(false);
}

UUserWidget* ADemoMaidPlayerController::EnsureWidget(
	TObjectPtr<UUserWidget>& WidgetInstance,
	const TSubclassOf<UUserWidget> WidgetClass
)
{
	if (WidgetInstance)
	{
		return WidgetInstance;
	}

	if (!WidgetClass)
	{
		return nullptr;
	}

	WidgetInstance = CreateWidget<UUserWidget>(this, WidgetClass);
	return WidgetInstance;
}

void ADemoMaidPlayerController::SetGameplayInputEnabled(const bool bEnabled)
{
	ResetIgnoreMoveInput();
	ResetIgnoreLookInput();
	SetIgnoreMoveInput(!bEnabled);
	SetIgnoreLookInput(!bEnabled);
	if (APawn* InputPawn = GetPawn())
	{
		if (bEnabled)
		{
			InputPawn->EnableInput(this);
		}
		else
		{
			InputPawn->DisableInput(this);
		}
	}
}

bool ADemoMaidPlayerController::CanAcceptMenuInput() const
{
	const AMeidoDemoGameMode* MenuFlow = GetDemoGameMode();
	return MenuFlow && !MenuFlow->IsWaitingForMenuCamera() && !MenuFlow->IsWaitingForWinSequence()
		&& !MenuFlow->IsWaitingForLoseSequence() && ActiveMenuKind != EDemoMenuKind::None;
}

bool ADemoMaidPlayerController::CanPresentTutorial() const
{
	return TutorialWidgetClass != nullptr;
}

void ADemoMaidPlayerController::SelectMenuEntry(const int32 InMenuIndex)
{
	if (!CanAcceptMenuInput() || InMenuIndex < 0 || InMenuIndex >= GetMenuOptionCount() || MenuSelectedIndex == InMenuIndex)
	{
		return;
	}
	MenuSelectedIndex = InMenuIndex;
	PlayFlowSound(FlowAudio ? FlowAudio->Navigate.Get() : nullptr);
	BroadcastMenuSelection();
}

void ADemoMaidPlayerController::ActivateMenuEntry(const int32 InMenuIndex)
{
	if (!CanAcceptMenuInput() || InMenuIndex < 0 || InMenuIndex >= GetMenuOptionCount())
	{
		return;
	}
	MenuSelectedIndex = InMenuIndex;
	BroadcastMenuSelection();
	ConfirmMenuSelection();
}

void ADemoMaidPlayerController::AdvanceTutorial()
{
	if (!CanAcceptMenuInput() || GetFlowState() != EFlowState::Tutorial)
	{
		return;
	}
	if (AMeidoDemoGameMode* TutorialFlow = GetDemoGameMode())
	{
		PlayFlowSound(FlowAudio ? FlowAudio->Confirm.Get() : nullptr);
		TutorialFlow->RequestNextTutorialSlide();
	}
}

void ADemoMaidPlayerController::PreviousTutorialSlide()
{
	if (!CanAcceptMenuInput() || GetFlowState() != EFlowState::Tutorial)
	{
		return;
	}
	if (AMeidoDemoGameMode* TutorialFlow = GetDemoGameMode())
	{
		if (TutorialFlow->GetTutorialSlideIndex() > 0)
		{
			PlayFlowSound(FlowAudio ? FlowAudio->Cancel.Get() : nullptr);
			TutorialFlow->RequestPreviousTutorialSlide();
		}
	}
}

void ADemoMaidPlayerController::ShowTutorialUI()
{
	HideAllFlowWidgets();
	if (!TutorialWidget && TutorialWidgetClass)
	{
		TutorialWidget = CreateWidget<UMeidoTutorialWidget>(this, TutorialWidgetClass);
	}
	if (TutorialWidget)
	{
		TutorialWidget->AddToViewport(150);
	}
	FInputModeGameAndUI TutorialInputMode;
	TutorialInputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	TutorialInputMode.SetHideCursorDuringCapture(false);
	if (TutorialWidget)
	{
		TutorialInputMode.SetWidgetToFocus(TutorialWidget->TakeWidget());
	}
	SetInputMode(TutorialInputMode);
	bShowMouseCursor = true;
	SetPause(false);
	SetGameplayInputEnabled(false);
	SetAllEnemyAIEnabled(false);
}

void ADemoMaidPlayerController::SetGameplayHUDVisible(const bool bInVisible)
{
	if (bInVisible && !GameplayHUD && GameplayHUDClass && IsLocalPlayerController())
	{
		GameplayHUD = CreateWidget<UMeidoHUD>(this, GameplayHUDClass);
	}
	if (GameplayHUD)
	{
		if (bInVisible)
		{
			if (!GameplayHUD->IsInViewport())
			{
				GameplayHUD->AddToViewport(0);
			}
			GameplayHUD->BindToPawn(GetPawn());
			GameplayHUD->BindToDemoFlow(GetDemoGameMode());
		}
		GameplayHUD->SetVisibility(bInVisible ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}
}

void ADemoMaidPlayerController::PlayFlowSound(USoundBase* InFlowSound) const
{
	if (InFlowSound && IsLocalPlayerController())
	{
		// UI sound remains audible while paused. No asset path is hardcoded.
		UGameplayStatics::PlaySound2D(this, InFlowSound, 1.f, 1.f, 0.f, nullptr, nullptr, true);
	}
}

void ADemoMaidPlayerController::SetAllEnemyAIEnabled(const bool bEnabled)
{
	const bool bFinalEnabled = bEnabled && !bDebugDisableEnemyAI;

	for (TActorIterator<AEnemyMaid> It(GetWorld()); It; ++It)
	{
		SetEnemyAIEnabled(*It, bFinalEnabled);
	}
}

void ADemoMaidPlayerController::SetEnemyAIEnabledPublic(AEnemyMaid* Enemy, const bool bEnabled)
{
	SetEnemyAIEnabled(Enemy, bEnabled && !bDebugDisableEnemyAI);
}

void ADemoMaidPlayerController::SetEnemyAIEnabled(AEnemyMaid* Enemy, const bool bEnabled)
{
	if (!Enemy)
	{
		return;
	}

	AEnemyMaidAIController* EnemyController = Cast<AEnemyMaidAIController>(Enemy->GetController());
	if (!EnemyController)
	{
		if (bEnabled)
		{
			Enemy->SpawnDefaultController();
			EnemyController = Cast<AEnemyMaidAIController>(Enemy->GetController());
		}
	}

	if (EnemyController)
	{
		EnemyController->SetAIEnabled(bEnabled);
	}
}
