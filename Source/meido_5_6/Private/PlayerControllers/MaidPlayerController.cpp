// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerControllers/MaidPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameModes/DemoGameMode.h"
#include "InputAction.h"
#include "InputCoreTypes.h"

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
			PauseInputAction->bTriggerWhenPaused = true;
			EnhancedInputComponent->BindAction(PauseInputAction, ETriggerEvent::Started, this, &AMaidPlayerController::OnPauseInput);
		}
	}

	if (InputComponent)
	{
		// TODO remove (debug only)
		InputComponent->BindKey(EKeys::K, IE_Pressed, this, &AMaidPlayerController::OnDebugToggleEnemyAI);
	}
}

void AMaidPlayerController::OnPauseInput()
{
	GetWorld()->GetAuthGameMode<ADemoGameMode>()->TogglePause();
}

void AMaidPlayerController::OnDebugToggleEnemyAI()
{
	GetWorld()->GetAuthGameMode<ADemoGameMode>()->ToggleDebugEnemyAI();
}
