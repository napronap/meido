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
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void ADemoMaidPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (AMeidoDemoGameMode* GM = GetDemoGameMode())
	{
		GM->OnFlowStateChanged.RemoveDynamic(this, &ADemoMaidPlayerController::HandleGameModeFlowChanged);
		GM->OnFlowStateChanged.AddDynamic(this, &ADemoMaidPlayerController::HandleGameModeFlowChanged);
		// GM EnterMainMenu may already have fired before PC BeginPlay — apply current state.
		ApplyFlowPresentation(GM->GetFlowState());
	}
	else
	{
		// Map missing MeidoDemoGameMode: still show main menu UI so BP maps don't go blank.
		ShowMainMenuUI();
	}
}

void ADemoMaidPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (IsLocalPlayerController())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			if (DefaultIMC)
			{
				Subsystem->AddMappingContext(DefaultIMC, 0);
			}
		}
	}

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (PauseInputAction)
		{
			PauseInputAction->bTriggerWhenPaused = true;
			EnhancedInputComponent->BindAction(
				PauseInputAction,
				ETriggerEvent::Started,
				this,
				&ADemoMaidPlayerController::OnPauseInput
			);
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
	}
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
	// Presentation already applied by GM via ApplyFlowPresentation; avoid double if GM calls Apply then broadcasts.
	// GM SetFlowState: Broadcast then ApplyFlowPresentation — so we only rebroadcast for BP here.
	// Wait: GM does ApplyFlowPresentation inside SetFlowState AFTER broadcast. Order is:
	//   FlowState = New; OnFlowStateChanged.Broadcast; PC->ApplyFlowPresentation
	// PC is also listening to OnFlowStateChanged — would Apply twice if we Apply here.
	// So HandleGameModeFlowChanged only rebroadcasts for local BP listeners.
}

void ADemoMaidPlayerController::StartGameFromMenu()
{
	if (AMeidoDemoGameMode* GM = GetDemoGameMode())
	{
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
	if (AMeidoDemoGameMode* GM = GetDemoGameMode())
	{
		GM->RequestQuit();
		return;
	}
	UKismetSystemLibrary::QuitGame(this, this, EQuitPreference::Quit, false);
}

void ADemoMaidPlayerController::RestartCurrentLevel()
{
	if (AMeidoDemoGameMode* GM = GetDemoGameMode())
	{
		GM->RequestRestartRun();
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

void ADemoMaidPlayerController::ApplyFlowPresentation(const EFlowState NewState)
{
	switch (NewState)
	{
	case EFlowState::MainMenu:
	case EFlowState::Tutorial:
		ShowMainMenuUI();
		break;
	case EFlowState::Playing:
		ShowPlayingUI();
		break;
	case EFlowState::Paused:
		ShowPausedUI();
		break;
	case EFlowState::Win:
		ShowWinUI();
		break;
	case EFlowState::Lose:
		ShowLoseUI();
		break;
	default:
		break;
	}
}

void ADemoMaidPlayerController::PrepareLocalRestartPresentation()
{
	HideAllFlowWidgets();
	UGameplayStatics::SetGlobalTimeDilation(this, 1.0f);
	SetPause(false);
	SetGameplayInputEnabled(false);
	SetAllEnemyAIEnabled(false);
}

void ADemoMaidPlayerController::ApplyRestartInterimPresentation()
{
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
	bShowMouseCursor = false;
	SetGameplayInputEnabled(false);
	SetAllEnemyAIEnabled(false);
}

void ADemoMaidPlayerController::ApplyPreWinPresentation()
{
	SetPause(false);
	SetGameplayInputEnabled(false);
	SetAllEnemyAIEnabled(false);
}

void ADemoMaidPlayerController::ApplyPreLosePresentation()
{
	SetPause(false);
	SetGameplayInputEnabled(false);
	SetAllEnemyAIEnabled(false);
}

void ADemoMaidPlayerController::HideAllFlowWidgets()
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

	FInputModeUIOnly InputMode;
	if (MainMenuWidget)
	{
		InputMode.SetWidgetToFocus(MainMenuWidget->TakeWidget());
	}
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
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

	FInputModeUIOnly InputMode;
	if (WinWidget)
	{
		InputMode.SetWidgetToFocus(WinWidget->TakeWidget());
	}
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
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

	FInputModeUIOnly InputMode;
	if (LoseWidget)
	{
		InputMode.SetWidgetToFocus(LoseWidget->TakeWidget());
	}
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
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
