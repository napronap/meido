// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerControllers/MaidPlayerController.h"
#include "Characters/MaidCharacter.h"
#include "Characters/PlayerMaidCharacter.h"
#include "Characters/EnemyMaid.h"
#include "Spawners/EnemyWaveSpawner.h"
#include "ActorComponents/HealthComponent.h"
#include "IA/EnemyMaidAIController.h"
#include "Blueprint/UserWidget.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "Engine/LocalPlayer.h"
#include "EngineUtils.h"
#include "Components/InputComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"

void AMaidPlayerController::BeginPlay()
{
	Super::BeginPlay();

	BindFlowSources();
	EnterMainMenuState();
}

void AMaidPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (IsLocalPlayerController())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultIMC, 0);
		}
	}

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (PauseInputAction)
		{
			// Allow this action to fire while gameplay is paused so it can unpause.
			PauseInputAction->bTriggerWhenPaused = true;
			EnhancedInputComponent->BindAction(PauseInputAction, ETriggerEvent::Started, this, &AMaidPlayerController::OnPauseInput);
		}
	}
}

void AMaidPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	BindPlayerHealth(InPawn);
}

void AMaidPlayerController::OnUnPossess()
{
	BindPlayerHealth(nullptr);
	Super::OnUnPossess();
}

void AMaidPlayerController::StartGameFromMenu()
{
	const bool bRestartingFromResultState = CurrentFlowState == EFlowState::Win
		|| CurrentFlowState == EFlowState::Lose;

	const bool bCanStartFlow = CurrentFlowState == EFlowState::MainMenu
		|| CurrentFlowState == EFlowState::Win
		|| CurrentFlowState == EFlowState::Lose;
	if (!bCanStartFlow)
	{
		return;
	}

	if (bWaitingForMenuCameraTransition)
	{
		return;
	}

	if (CurrentFlowState == EFlowState::Win || CurrentFlowState == EFlowState::Lose)
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

		bWaitingForWinSequence = false;
		bWaitingForLoseSequence = false;
		UGameplayStatics::SetGlobalTimeDilation(this, 1.0f);
		SetPause(false);
		SetGameplayInputEnabled(false);
		SetAllEnemyAIEnabled(false);

		if (AMaidCharacter* MaidPawn = Cast<AMaidCharacter>(GetPawn()))
		{
			MaidPawn->CustomTimeDilation = 1.0f;
			MaidPawn->ResetForFlowRestart();
		}

		if (BoundWaveSpawner)
		{
			BoundWaveSpawner->ResetWave();
			BoundWaveSpawner->StartWave();
		}

		FInputModeGameOnly InputMode;
		SetInputMode(InputMode);
		bShowMouseCursor = false;
		SetFlowState(EFlowState::MainMenu);
	}

	if (APlayerMaidCharacter* PlayerMaid = Cast<APlayerMaidCharacter>(GetPawn()))
	{
		// For restart from Win/Lose, interpolate directly from current camera (no snap to menu pose).
		if (!bRestartingFromResultState)
		{
			PlayerMaid->ApplyMenuCameraPose();
		}
		PlayerMaid->StartMenuCameraTransitionToGameplay();
		if (PlayerMaid->IsMenuCameraTransitionActive())
		{
			bWaitingForMenuCameraTransition = true;
			return;
		}
	}

	CompleteStartGameFromMenu();
}

void AMaidPlayerController::CompleteStartGameFromMenu()
{
	if (CurrentFlowState != EFlowState::MainMenu)
	{
		return;
	}

	bWaitingForMenuCameraTransition = false;
	EnterPlayingState();
}

void AMaidPlayerController::CompleteWinSequence()
{
	if (!bWaitingForWinSequence)
	{
		return;
	}

	bWaitingForWinSequence = false;

	if (CurrentFlowState == EFlowState::Playing || CurrentFlowState == EFlowState::Paused)
	{
		EnterWinState();
	}
}

void AMaidPlayerController::CompleteLoseSequence()
{
	if (!bWaitingForLoseSequence)
	{
		return;
	}

	UGameplayStatics::SetGlobalTimeDilation(this, 1.0f);
	if (AMaidCharacter* MaidPawn = Cast<AMaidCharacter>(GetPawn()))
	{
		MaidPawn->CustomTimeDilation = 1.0f;
	}
	ClearEnemiesAfterLoseTransition();

	if (CurrentFlowState == EFlowState::Playing || CurrentFlowState == EFlowState::Paused)
	{
		EnterLoseState();
	}

	bWaitingForLoseSequence = false;
}

void AMaidPlayerController::ResumeGameFromPause()
{
	if (CurrentFlowState != EFlowState::Paused)
	{
		return;
	}

	EnterPlayingState();
}

void AMaidPlayerController::QuitGameRequested()
{
	UKismetSystemLibrary::QuitGame(this, this, EQuitPreference::Quit, false);
}

void AMaidPlayerController::RestartCurrentLevel()
{
	StartGameFromMenu();
}

void AMaidPlayerController::TogglePauseMenu()
{
	if (bWaitingForWinSequence || bWaitingForLoseSequence)
	{
		return;
	}

	if (CurrentFlowState != EFlowState::Playing && CurrentFlowState != EFlowState::Paused)
	{
		return;
	}

	if (CurrentFlowState == EFlowState::Playing)
	{
		EnterPausedState();
		return;
	}

	if (CurrentFlowState == EFlowState::Paused)
	{
		ResumeGameFromPause();
	}
}

void AMaidPlayerController::OnPauseInput(const FInputActionValue& Value)
{
	(void)Value;
	TogglePauseMenu();
}

void AMaidPlayerController::EnterMainMenuState()
{
	if (UUserWidget* Widget = EnsureWidget(MainMenuWidget, MainMenuWidgetClass))
	{
		Widget->AddToViewport(100);
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
	bWaitingForMenuCameraTransition = false;
	bWaitingForWinSequence = false;
	bWaitingForLoseSequence = false;
	SetGameplayInputEnabled(false);
	SetAllEnemyAIEnabled(false);
	SetFlowState(EFlowState::MainMenu);
}

void AMaidPlayerController::EnterPlayingState()
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

	SetPause(false);

	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
	bShowMouseCursor = false;
	bWaitingForWinSequence = false;
	bWaitingForLoseSequence = false;
	SetGameplayInputEnabled(true);
	SetAllEnemyAIEnabled(true);
	SetFlowState(EFlowState::Playing);

	if (BoundWaveSpawner && BoundWaveSpawner->IsWaveCompleted())
	{
		StartWinSequence();
		return;
	}

	if (BoundPlayerHealthComponent && BoundPlayerHealthComponent->IsDead())
	{
		EnterLoseState();
	}
}

void AMaidPlayerController::EnterPausedState()
{
	if (UUserWidget* Widget = EnsureWidget(PauseMenuWidget, PauseMenuWidgetClass))
	{
		Widget->AddToViewport(200);
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
	bWaitingForWinSequence = false;
	bWaitingForLoseSequence = false;
	SetPause(true);
	SetGameplayInputEnabled(false);
	SetFlowState(EFlowState::Paused);
}

void AMaidPlayerController::StartWinSequence()
{
	if (CurrentFlowState != EFlowState::Playing && CurrentFlowState != EFlowState::Paused)
	{
		return;
	}

	if (bWaitingForWinSequence)
	{
		return;
	}

	bWaitingForWinSequence = false;
	bWaitingForLoseSequence = false;
	UGameplayStatics::SetGlobalTimeDilation(this, 1.0f);
	if (AMaidCharacter* MaidPawn = Cast<AMaidCharacter>(GetPawn()))
	{
		MaidPawn->CustomTimeDilation = 1.0f;
	}
	SetPause(false);
	SetGameplayInputEnabled(false);
	SetAllEnemyAIEnabled(false);

	if (APlayerMaidCharacter* PlayerMaid = Cast<APlayerMaidCharacter>(GetPawn()))
	{
		PlayerMaid->StartWinSequenceCameraTransition();
		if (PlayerMaid->IsWinSequenceActive())
		{
			bWaitingForWinSequence = true;
			return;
		}
	}

	EnterWinState();
}

void AMaidPlayerController::StartLoseSequence()
{
	if (CurrentFlowState != EFlowState::Playing && CurrentFlowState != EFlowState::Paused)
	{
		return;
	}

	if (bWaitingForLoseSequence)
	{
		return;
	}

	bWaitingForWinSequence = false;
	bWaitingForLoseSequence = false;
	SetPause(false);
	SetGameplayInputEnabled(false);
	SetAllEnemyAIEnabled(false);

	if (BoundWaveSpawner)
	{
		// Prevent new spawns during lose camera transition.
		BoundWaveSpawner->StopWave();
	}

	if (APlayerMaidCharacter* PlayerMaid = Cast<APlayerMaidCharacter>(GetPawn()))
	{
		PlayerMaid->StartLoseSequenceCameraTransition();
		if (PlayerMaid->IsLoseSequenceActive())
		{
			bWaitingForLoseSequence = true;
			return;
		}
	}

	ClearEnemiesAfterLoseTransition();
	EnterLoseState();
}

void AMaidPlayerController::ClearEnemiesAfterLoseTransition()
{
	if (BoundWaveSpawner)
	{
		BoundWaveSpawner->ResetWave();
	}

	TArray<AEnemyMaid*> EnemiesToDestroy;
	for (TActorIterator<AEnemyMaid> It(GetWorld()); It; ++It)
	{
		if (AEnemyMaid* Enemy = *It)
		{
			EnemiesToDestroy.Add(Enemy);
		}
	}

	for (AEnemyMaid* Enemy : EnemiesToDestroy)
	{
		if (Enemy)
		{
			Enemy->Destroy();
		}
	}
}

void AMaidPlayerController::EnterWinState()
{
	if (CurrentFlowState == EFlowState::Win)
	{
		return;
	}

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
		Widget->AddToViewport(300);
	}

	FInputModeUIOnly InputMode;
	if (WinWidget)
	{
		InputMode.SetWidgetToFocus(WinWidget->TakeWidget());
	}
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);

	bShowMouseCursor = true;
	bWaitingForWinSequence = false;
	bWaitingForLoseSequence = false;
	SetPause(false);
	SetGameplayInputEnabled(false);
	SetAllEnemyAIEnabled(false);
	SetFlowState(EFlowState::Win);
}

void AMaidPlayerController::EnterLoseState()
{
	if (CurrentFlowState == EFlowState::Lose)
	{
		return;
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

	if (UUserWidget* Widget = EnsureWidget(LoseWidget, LoseWidgetClass))
	{
		Widget->AddToViewport(300);
	}

	FInputModeUIOnly InputMode;
	if (LoseWidget)
	{
		InputMode.SetWidgetToFocus(LoseWidget->TakeWidget());
	}
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);

	bShowMouseCursor = true;
	bWaitingForWinSequence = false;
	bWaitingForLoseSequence = false;
	SetPause(false);
	SetGameplayInputEnabled(false);
	SetAllEnemyAIEnabled(false);
	SetFlowState(EFlowState::Lose);
}

void AMaidPlayerController::SetFlowState(const EFlowState NewState)
{
	CurrentFlowState = NewState;
	OnFlowStateChanged.Broadcast(CurrentFlowState);
}

UUserWidget* AMaidPlayerController::EnsureWidget(TObjectPtr<UUserWidget>& WidgetInstance, const TSubclassOf<UUserWidget> WidgetClass)
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

void AMaidPlayerController::SetGameplayInputEnabled(const bool bEnabled)
{
	// These ignore flags are stack-based in UE, so force a deterministic state first.
	ResetIgnoreMoveInput();
	ResetIgnoreLookInput();

	SetIgnoreMoveInput(!bEnabled);
	SetIgnoreLookInput(!bEnabled);
}

void AMaidPlayerController::BindFlowSources()
{
	if (!BoundWaveSpawner)
	{
		for (TActorIterator<AEnemyWaveSpawner> It(GetWorld()); It; ++It)
		{
			BoundWaveSpawner = *It;
			break;
		}
	}

	if (BoundWaveSpawner)
	{
		BoundWaveSpawner->OnEnemySpawned.RemoveDynamic(this, &AMaidPlayerController::HandleEnemySpawned);
		BoundWaveSpawner->OnEnemySpawned.AddDynamic(this, &AMaidPlayerController::HandleEnemySpawned);
		BoundWaveSpawner->OnWaveCompleted.RemoveDynamic(this, &AMaidPlayerController::HandleWaveCompleted);
		BoundWaveSpawner->OnWaveCompleted.AddDynamic(this, &AMaidPlayerController::HandleWaveCompleted);
	}

	BindPlayerHealth(GetPawn());
}

void AMaidPlayerController::BindPlayerHealth(APawn* InPawn)
{
	if (BoundPlayerHealthComponent)
	{
		BoundPlayerHealthComponent->OnHealthDepleted.RemoveDynamic(
			this,
			&AMaidPlayerController::HandlePlayerHealthDepleted
		);
		BoundPlayerHealthComponent = nullptr;
	}

	if (!InPawn)
	{
		return;
	}

	if (UHealthComponent* HealthComponent = InPawn->FindComponentByClass<UHealthComponent>())
	{
		BoundPlayerHealthComponent = HealthComponent;
		BoundPlayerHealthComponent->OnHealthDepleted.RemoveDynamic(
			this,
			&AMaidPlayerController::HandlePlayerHealthDepleted
		);
		BoundPlayerHealthComponent->OnHealthDepleted.AddDynamic(
			this,
			&AMaidPlayerController::HandlePlayerHealthDepleted
		);
	}
}

void AMaidPlayerController::HandleWaveCompleted()
{
	if (bWaitingForLoseSequence || CurrentFlowState == EFlowState::Lose)
	{
		return;
	}

	if (CurrentFlowState != EFlowState::Playing && CurrentFlowState != EFlowState::Paused)
	{
		return;
	}

	StartWinSequence();
}

void AMaidPlayerController::HandleEnemySpawned(AEnemyMaid* SpawnedEnemy)
{
	const bool bShouldEnableAI = (CurrentFlowState == EFlowState::Playing) && !bWaitingForWinSequence;
	SetEnemyAIEnabled(SpawnedEnemy, bShouldEnableAI);
}

void AMaidPlayerController::HandlePlayerHealthDepleted(UHealthComponent* InHealthComponent, AActor* DamageCauser)
{
	(void)InHealthComponent;
	(void)DamageCauser;

	if (CurrentFlowState != EFlowState::Playing && CurrentFlowState != EFlowState::Paused)
	{
		return;
	}

	bWaitingForWinSequence = false;
	StartLoseSequence();
}

void AMaidPlayerController::SetAllEnemyAIEnabled(const bool bEnabled)
{
	for (TActorIterator<AEnemyMaid> It(GetWorld()); It; ++It)
	{
		SetEnemyAIEnabled(*It, bEnabled);
	}
}

void AMaidPlayerController::SetEnemyAIEnabled(AEnemyMaid* Enemy, const bool bEnabled)
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
