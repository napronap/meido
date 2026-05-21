// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerControllers/MaidPlayerController.h"

#include "EnhancedInputSubsystems.h"

void AMaidPlayerController::BeginPlay()
{
	Super::BeginPlay();
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
}
