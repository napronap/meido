// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerControllers/MaidPlayerController.h"

#include "Blueprint/UserWidget.h"
#include "Characters/EnemyMaid.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"
#include "EngineUtils.h"
#include "IA/EnemyMaidAIController.h"
#include "InputAction.h"
#include "InputCoreTypes.h"
#include "UI/MeidoHUD.h"

void AMaidPlayerController::BeginPlay()
{
	Super::BeginPlay();
	RefreshGameplayHUD();
}

void AMaidPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	RefreshGameplayHUD();
}

void AMaidPlayerController::OnUnPossess()
{
	if (GameplayHUD)
	{
		GameplayHUD->BindToPawn(nullptr);
	}
	Super::OnUnPossess();
}

void AMaidPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GameplayHUD)
	{
		GameplayHUD->RemoveFromParent();
		GameplayHUD = nullptr;
	}
	if (PauseMenuWidget)
	{
		PauseMenuWidget->RemoveFromParent();
	}
	Super::EndPlay(EndPlayReason);
}

void AMaidPlayerController::RefreshGameplayHUD()
{
	if (!IsLocalPlayerController())
	{
		return;
	}
	if (!GameplayHUD && GameplayHUDClass)
	{
		GameplayHUD = CreateWidget<UMeidoHUD>(this, GameplayHUDClass);
	}
	if (GameplayHUD)
	{
		if (!GameplayHUD->IsInViewport())
		{
			GameplayHUD->AddToViewport(0);
		}
		GameplayHUD->BindToPawn(GetPawn());
		GameplayHUD->BindToDemoFlow(nullptr);
	}
}

void AMaidPlayerController::SetupInputComponent()
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

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (PauseInputAction)
		{
			// Must fire while world is paused so the same key can resume.
			PauseInputAction->bTriggerWhenPaused = true;
			EIC->BindAction(
				PauseInputAction,
				ETriggerEvent::Started,
				this,
				&AMaidPlayerController::OnPauseInput
			);
		}
	}

	if (InputComponent)
	{
		// TODO remove (debug only) — POST 4.b
		InputComponent->BindKey(EKeys::K, IE_Pressed, this, &AMaidPlayerController::OnDebugToggleEnemyAI);
	}
}

void AMaidPlayerController::OnPauseInput(const FInputActionValue& Value)
{
	(void)Value;
	TogglePauseMenu();
}

void AMaidPlayerController::TogglePauseMenu()
{
	if (bPlaygroundPaused)
	{
		ResumeFromPause();
	}
	else
	{
		EnterPaused();
	}
}

void AMaidPlayerController::ResumeFromPause()
{
	if (!bPlaygroundPaused)
	{
		return;
	}
	LeavePaused();
}

void AMaidPlayerController::EnterPaused()
{
	if (bPlaygroundPaused)
	{
		return;
	}

	bPlaygroundPaused = true;

	if (PauseMenuWidgetClass)
	{
		if (!PauseMenuWidget)
		{
			PauseMenuWidget = CreateWidget<UUserWidget>(this, PauseMenuWidgetClass);
		}
		if (PauseMenuWidget && !PauseMenuWidget->IsInViewport())
		{
			PauseMenuWidget->AddToViewport(200);
		}
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
}

void AMaidPlayerController::LeavePaused()
{
	if (!bPlaygroundPaused)
	{
		return;
	}

	bPlaygroundPaused = false;

	if (PauseMenuWidget && PauseMenuWidget->IsInViewport())
	{
		PauseMenuWidget->RemoveFromParent();
	}

	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
	bShowMouseCursor = false;

	SetPause(false);
}

void AMaidPlayerController::OnDebugToggleEnemyAI()
{
	bDebugDisableEnemyAI = !bDebugDisableEnemyAI;
	SetAllEnemyAIEnabled(!bDebugDisableEnemyAI);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			1.5f,
			bDebugDisableEnemyAI ? FColor::Orange : FColor::Green,
			bDebugDisableEnemyAI ? TEXT("Enemy AI: OFF (K)") : TEXT("Enemy AI: ON (K)")
		);
	}
}

void AMaidPlayerController::SetAllEnemyAIEnabled(const bool bEnabled)
{
	const bool bFinalEnabled = bEnabled && !bDebugDisableEnemyAI;

	for (TActorIterator<AEnemyMaid> It(GetWorld()); It; ++It)
	{
		SetEnemyAIEnabled(*It, bFinalEnabled);
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
