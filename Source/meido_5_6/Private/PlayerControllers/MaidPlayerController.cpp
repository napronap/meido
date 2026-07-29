// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerControllers/MaidPlayerController.h"

#include "Characters/EnemyMaid.h"
#include "Components/InputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "IA/EnemyMaidAIController.h"
#include "InputCoreTypes.h"

void AMaidPlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void AMaidPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (IsLocalPlayerController())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultIMC, 0);
		}
	}

	if (InputComponent)
	{
		// TODO remove (debug only): same K freeze as DemoMaidPlayerController — see POST_REFACTOR_NOTES.
		InputComponent->BindKey(EKeys::K, IE_Pressed, this, &AMaidPlayerController::OnDebugToggleEnemyAI);
	}
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
