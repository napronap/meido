// Fill out your copyright notice in the Description page of Project Settings.

#include "GameModes/MeidoDemoGameMode.h"

#include "ActorComponents/HealthComponent.h"
#include "Characters/DemoPlayerMaidCharacter.h"
#include "Characters/EnemyMaid.h"
#include "Characters/MaidCharacter.h"
#include "EngineUtils.h"
#include "IA/EnemyMaidAIController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "PlayerControllers/DemoMaidPlayerController.h"
#include "Spawners/EnemyWaveSpawner.h"

AMeidoDemoGameMode::AMeidoDemoGameMode()
{
	PlayerControllerClass = ADemoMaidPlayerController::StaticClass();
	// DefaultPawnClass: set on BP GameMode or World Settings (DemoPlayer BP).
}

void AMeidoDemoGameMode::BeginPlay()
{
	Super::BeginPlay();

	BindFlowSources();
	EnterMainMenu();
}

void AMeidoDemoGameMode::BindFlowSources()
{
	if (bAutoFindWaveSpawner && !BoundWaveSpawner)
	{
		for (TActorIterator<AEnemyWaveSpawner> It(GetWorld()); It; ++It)
		{
			BoundWaveSpawner = *It;
			break;
		}
	}

	if (BoundWaveSpawner)
	{
		BoundWaveSpawner->OnEnemySpawned.RemoveDynamic(this, &AMeidoDemoGameMode::HandleEnemySpawned);
		BoundWaveSpawner->OnEnemySpawned.AddDynamic(this, &AMeidoDemoGameMode::HandleEnemySpawned);
		BoundWaveSpawner->OnWaveCompleted.RemoveDynamic(this, &AMeidoDemoGameMode::HandleWaveCompleted);
		BoundWaveSpawner->OnWaveCompleted.AddDynamic(this, &AMeidoDemoGameMode::HandleWaveCompleted);
	}

	BindPlayerHealthFromPawn(GetPlayerPawn());
}

void AMeidoDemoGameMode::BindPlayerHealthFromPawn(APawn* Pawn)
{
	if (BoundPlayerHealth)
	{
		BoundPlayerHealth->OnHealthDepleted.RemoveDynamic(this, &AMeidoDemoGameMode::HandlePlayerHealthDepleted);
		BoundPlayerHealth = nullptr;
	}

	if (!Pawn)
	{
		return;
	}

	if (UHealthComponent* Health = Pawn->FindComponentByClass<UHealthComponent>())
	{
		BoundPlayerHealth = Health;
		BoundPlayerHealth->OnHealthDepleted.RemoveDynamic(this, &AMeidoDemoGameMode::HandlePlayerHealthDepleted);
		BoundPlayerHealth->OnHealthDepleted.AddDynamic(this, &AMeidoDemoGameMode::HandlePlayerHealthDepleted);
	}
}

ADemoMaidPlayerController* AMeidoDemoGameMode::GetDemoPC() const
{
	return Cast<ADemoMaidPlayerController>(UGameplayStatics::GetPlayerController(this, 0));
}

APawn* AMeidoDemoGameMode::GetPlayerPawn() const
{
	if (ADemoMaidPlayerController* PC = GetDemoPC())
	{
		return PC->GetPawn();
	}
	return UGameplayStatics::GetPlayerPawn(this, 0);
}

AMaidCharacter* AMeidoDemoGameMode::GetPlayerMaid() const
{
	return Cast<AMaidCharacter>(GetPlayerPawn());
}

void AMeidoDemoGameMode::SetFlowState(const EFlowState NewState)
{
	if (FlowState == NewState)
	{
		return;
	}

	FlowState = NewState;
	OnFlowStateChanged.Broadcast(FlowState);

	if (ADemoMaidPlayerController* PC = GetDemoPC())
	{
		PC->ApplyFlowPresentation(FlowState);
	}
}

void AMeidoDemoGameMode::EnterMainMenu()
{
	bWaitingForMenuCameraTransition = false;
	bWaitingForWinSequence = false;
	bWaitingForLoseSequence = false;
	SetFlowState(EFlowState::MainMenu);
}

void AMeidoDemoGameMode::EnterPlaying()
{
	bWaitingForWinSequence = false;
	bWaitingForLoseSequence = false;
	bWaitingForMenuCameraTransition = false;
	SetFlowState(EFlowState::Playing);

	// Re-bind health after restart (pawn may have been reset).
	BindPlayerHealthFromPawn(GetPlayerPawn());

	if (BoundWaveSpawner && BoundWaveSpawner->IsWaveCompleted())
	{
		StartWinSequence();
		return;
	}

	if (BoundPlayerHealth && BoundPlayerHealth->IsDead())
	{
		EnterLose();
	}
}

void AMeidoDemoGameMode::EnterPaused()
{
	bWaitingForWinSequence = false;
	bWaitingForLoseSequence = false;
	SetFlowState(EFlowState::Paused);
}

void AMeidoDemoGameMode::EnterWin()
{
	bWaitingForWinSequence = false;
	bWaitingForLoseSequence = false;
	SetFlowState(EFlowState::Win);
}

void AMeidoDemoGameMode::EnterLose()
{
	bWaitingForWinSequence = false;
	bWaitingForLoseSequence = false;
	SetFlowState(EFlowState::Lose);
}

void AMeidoDemoGameMode::PrepareRestartFromResult()
{
	if (ADemoMaidPlayerController* PC = GetDemoPC())
	{
		PC->PrepareLocalRestartPresentation();
	}

	UGameplayStatics::SetGlobalTimeDilation(this, 1.0f);

	if (AMaidCharacter* Maid = GetPlayerMaid())
	{
		Maid->CustomTimeDilation = 1.0f;
		Maid->ResetForFlowRestart();
	}

	if (BoundWaveSpawner)
	{
		BoundWaveSpawner->ResetWave();
		BoundWaveSpawner->StartWave();
	}

	// Flow state MainMenu so camera path / CompleteStart can run.
	bWaitingForWinSequence = false;
	bWaitingForLoseSequence = false;
	bWaitingForMenuCameraTransition = false;
	FlowState = EFlowState::MainMenu;
	OnFlowStateChanged.Broadcast(FlowState);
	// Do not ApplyFlowPresentation(MainMenu) here — would re-show menu mid-restart.
	if (ADemoMaidPlayerController* PC = GetDemoPC())
	{
		PC->ApplyRestartInterimPresentation();
	}
}

void AMeidoDemoGameMode::RequestStartFromMenu()
{
	const bool bRestartingFromResult =
		FlowState == EFlowState::Win || FlowState == EFlowState::Lose;

	const bool bCanStart = FlowState == EFlowState::MainMenu
		|| FlowState == EFlowState::Win
		|| FlowState == EFlowState::Lose;
	if (!bCanStart)
	{
		return;
	}

	if (bWaitingForMenuCameraTransition)
	{
		return;
	}

	if (bRestartingFromResult)
	{
		PrepareRestartFromResult();
	}

	if (ADemoPlayerMaidCharacter* DemoPlayer = Cast<ADemoPlayerMaidCharacter>(GetPlayerPawn()))
	{
		if (!bRestartingFromResult)
		{
			DemoPlayer->ApplyMenuCameraPose();
		}
		DemoPlayer->StartMenuCameraTransitionToGameplay();
		if (DemoPlayer->IsMenuCameraTransitionActive())
		{
			bWaitingForMenuCameraTransition = true;
			return;
		}
	}

	NotifyMenuCameraFinished();
}

void AMeidoDemoGameMode::NotifyMenuCameraFinished()
{
	if (FlowState != EFlowState::MainMenu)
	{
		return;
	}

	bWaitingForMenuCameraTransition = false;
	EnterPlaying();
}

void AMeidoDemoGameMode::NotifyPlayerPawnReady(APawn* Pawn)
{
	BindPlayerHealthFromPawn(Pawn);
}

void AMeidoDemoGameMode::NotifyWinCameraFinished()
{
	if (!bWaitingForWinSequence)
	{
		return;
	}

	bWaitingForWinSequence = false;

	if (FlowState == EFlowState::Playing || FlowState == EFlowState::Paused)
	{
		EnterWin();
	}
}

void AMeidoDemoGameMode::NotifyLoseCameraFinished()
{
	if (!bWaitingForLoseSequence)
	{
		return;
	}

	UGameplayStatics::SetGlobalTimeDilation(this, 1.0f);
	if (AMaidCharacter* Maid = GetPlayerMaid())
	{
		Maid->CustomTimeDilation = 1.0f;
	}
	ClearEnemiesAfterLose();

	if (FlowState == EFlowState::Playing || FlowState == EFlowState::Paused)
	{
		EnterLose();
	}

	bWaitingForLoseSequence = false;
}

void AMeidoDemoGameMode::RequestTogglePause()
{
	if (bWaitingForWinSequence || bWaitingForLoseSequence)
	{
		return;
	}

	if (FlowState == EFlowState::Playing)
	{
		EnterPaused();
		return;
	}

	if (FlowState == EFlowState::Paused)
	{
		RequestResumeFromPause();
	}
}

void AMeidoDemoGameMode::RequestResumeFromPause()
{
	if (FlowState != EFlowState::Paused)
	{
		return;
	}
	EnterPlaying();
}

void AMeidoDemoGameMode::RequestQuit()
{
	UKismetSystemLibrary::QuitGame(
		this,
		GetDemoPC(),
		EQuitPreference::Quit,
		false
	);
}

void AMeidoDemoGameMode::RequestRestartRun()
{
	RequestStartFromMenu();
}

void AMeidoDemoGameMode::StartWinSequence()
{
	if (FlowState != EFlowState::Playing && FlowState != EFlowState::Paused)
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
	if (AMaidCharacter* Maid = GetPlayerMaid())
	{
		Maid->CustomTimeDilation = 1.0f;
	}

	if (ADemoMaidPlayerController* PC = GetDemoPC())
	{
		PC->ApplyPreWinPresentation();
	}

	if (ADemoPlayerMaidCharacter* DemoPlayer = Cast<ADemoPlayerMaidCharacter>(GetPlayerPawn()))
	{
		DemoPlayer->StartWinSequenceCameraTransition();
		if (DemoPlayer->IsWinSequenceActive())
		{
			bWaitingForWinSequence = true;
			return;
		}
	}

	EnterWin();
}

void AMeidoDemoGameMode::StartLoseSequence()
{
	if (FlowState != EFlowState::Playing && FlowState != EFlowState::Paused)
	{
		return;
	}

	if (bWaitingForLoseSequence)
	{
		return;
	}

	bWaitingForWinSequence = false;
	bWaitingForLoseSequence = false;

	if (ADemoMaidPlayerController* PC = GetDemoPC())
	{
		PC->ApplyPreLosePresentation();
	}

	if (BoundWaveSpawner)
	{
		BoundWaveSpawner->StopWave();
	}

	if (ADemoPlayerMaidCharacter* DemoPlayer = Cast<ADemoPlayerMaidCharacter>(GetPlayerPawn()))
	{
		DemoPlayer->StartLoseSequenceCameraTransition();
		if (DemoPlayer->IsLoseSequenceActive())
		{
			bWaitingForLoseSequence = true;
			return;
		}
	}

	ClearEnemiesAfterLose();
	EnterLose();
}

void AMeidoDemoGameMode::ClearEnemiesAfterLose()
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

void AMeidoDemoGameMode::HandleWaveCompleted()
{
	if (bWaitingForLoseSequence || FlowState == EFlowState::Lose)
	{
		return;
	}

	if (FlowState != EFlowState::Playing && FlowState != EFlowState::Paused)
	{
		return;
	}

	StartWinSequence();
}

void AMeidoDemoGameMode::HandleEnemySpawned(AEnemyMaid* SpawnedEnemy)
{
	if (!SpawnedEnemy)
	{
		return;
	}

	// AI on only while actively playing (PC debug freeze still handled on PC).
	if (ADemoMaidPlayerController* PC = GetDemoPC())
	{
		const bool bEnable = (FlowState == EFlowState::Playing)
			&& !bWaitingForWinSequence
			&& !bWaitingForLoseSequence
			&& !PC->IsDebugEnemyAIDisabled();
		PC->SetEnemyAIEnabledPublic(SpawnedEnemy, bEnable);
	}
}

void AMeidoDemoGameMode::HandlePlayerHealthDepleted(
	UHealthComponent* /*InHealthComponent*/,
	AActor* /*DamageCauser*/
)
{
	if (bWaitingForLoseSequence || FlowState == EFlowState::Lose)
	{
		return;
	}

	if (FlowState != EFlowState::Playing && FlowState != EFlowState::Paused)
	{
		return;
	}

	StartLoseSequence();
}
