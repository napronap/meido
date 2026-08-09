// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerControllers/MeidoPlayerController.h"

#include "ActorComponents/HealthComponent.h"
#include "EnhancedInputSubsystems.h"
#include "UI/MeidoHUD.h"

void AMeidoPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalPlayerController())
	{
		ShowMeidoHUD();
	}
}

void AMeidoPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	HideMeidoHUD();
	Super::EndPlay(EndPlayReason);
}

void AMeidoPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (!IsLocalPlayerController())
	{
		return;
	}

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (DefaultIMC)
		{
			Subsystem->AddMappingContext(DefaultIMC, 0);
		}
	}
}

void AMeidoPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (IsLocalPlayerController())
	{
		if (!MeidoHUD)
		{
			ShowMeidoHUD();
		}
		else
		{
			BindHUDToPawn(InPawn);
		}
	}
}

void AMeidoPlayerController::OnUnPossess()
{
	if (MeidoHUD)
	{
		MeidoHUD->UnbindHealth();
	}

	Super::OnUnPossess();
}

void AMeidoPlayerController::ShowMeidoHUD()
{
	if (!IsLocalPlayerController())
	{
		return;
	}

	if (!MeidoHUD)
	{
		if (!MeidoHUDClass)
		{
			return;
		}

		MeidoHUD = CreateWidget<UMeidoHUD>(this, MeidoHUDClass);
		if (!MeidoHUD)
		{
			return;
		}
	}

	if (!MeidoHUD->IsInViewport())
	{
		MeidoHUD->AddToViewport(MeidoHUDZOrder);
	}

	BindHUDToPawn(GetPawn());
}

void AMeidoPlayerController::HideMeidoHUD()
{
	if (MeidoHUD)
	{
		MeidoHUD->UnbindHealth();
		MeidoHUD->RemoveFromParent();
		MeidoHUD = nullptr;
	}
}

void AMeidoPlayerController::BindHUDToPawn(APawn* InPawn)
{
	if (!MeidoHUD)
	{
		return;
	}

	UHealthComponent* Health = nullptr;
	if (InPawn)
	{
		Health = InPawn->FindComponentByClass<UHealthComponent>();
	}

	MeidoHUD->BindToHealth(Health);
}
