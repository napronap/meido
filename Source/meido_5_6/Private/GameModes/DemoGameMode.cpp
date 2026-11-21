#include "GameModes/DemoGameMode.h"
#include "ActorComponents/HealthComponent.h"
#include "Characters/EnemyMaid.h"
#include "Characters/PlayerMaidCharacter.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "IA/EnemyMaidAIController.h"
#include "Kismet/GameplayStatics.h"
#include "Spawners/EnemyWaveSpawner.h"

void ADemoGameMode::StartPlay()
{
	Super::StartPlay();

	PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	PlayerMaid = CastChecked<APlayerMaidCharacter>(PlayerController->GetPawn());
	PlayerMaid->GetHealthComponent()->OnHealthDepleted.AddDynamic(this, &ADemoGameMode::HandlePlayerHealthDepleted);

	// the level has exactly one spawner
	WaveSpawner = *TActorIterator<AEnemyWaveSpawner>(GetWorld());
	WaveSpawner->OnEnemySpawned.AddDynamic(this, &ADemoGameMode::HandleEnemySpawned);
	WaveSpawner->OnWaveCompleted.AddDynamic(this, &ADemoGameMode::HandleWaveCompleted);

	SetFlowState(EFlowState::MainMenu);
}

void ADemoGameMode::StartGame()
{
	// called from the main menu, or from Win / Lose to retry
	const bool bRetry = FlowState != EFlowState::MainMenu;
	if (bRetry)
	{
		// fresh player and wave, the camera goes to gameplay from where it is
		PlayerMaid->ResetForFlowRestart();
		WaveSpawner->ResetWave();
	}

	SetFlowState(EFlowState::Starting);
	PlayerMaid->StartMenuCameraTransitionToGameplay();
}

void ADemoGameMode::TogglePause()
{
	if (FlowState == EFlowState::Playing)
	{
		SetFlowState(EFlowState::Paused);
	}
	else if (FlowState == EFlowState::Paused)
	{
		SetFlowState(EFlowState::Playing);
	}
}

void ADemoGameMode::FinishTransition()
{
	switch (FlowState)
	{
	case EFlowState::Starting:
		SetFlowState(EFlowState::Playing);
		WaveSpawner->StartWave();
		return;

	case EFlowState::Winning:
		SetFlowState(EFlowState::Win);
		return;

	case EFlowState::Losing:
		// the enemies still alive leave with the lose screen
		WaveSpawner->ResetWave();
		for (TActorIterator<AEnemyMaid> Enemy(GetWorld()); Enemy; ++Enemy)
		{
			Enemy->Destroy();
		}
		SetFlowState(EFlowState::Lose);
		return;

	default:
		return;
	}
}

void ADemoGameMode::ToggleDebugEnemyAI()
{
	bDebugEnemyAIDisabled = !bDebugEnemyAIDisabled;
	SetAllEnemyAIEnabled();
}

void ADemoGameMode::SetFlowState(const EFlowState NewState)
{
	FlowState = NewState;

	const bool bPaused = FlowState == EFlowState::Paused;
	PlayerController->SetPause(bPaused);

	// SetIgnore* stacks a counter: reset first so it's an absolute on / off
	const bool bGameplayInput = FlowState == EFlowState::Playing;
	PlayerController->ResetIgnoreMoveInput();
	PlayerController->ResetIgnoreLookInput();
	PlayerController->SetIgnoreMoveInput(!bGameplayInput);
	PlayerController->SetIgnoreLookInput(!bGameplayInput);

	SetAllEnemyAIEnabled();

	OnFlowStateChanged.Broadcast(FlowState);
}

bool ADemoGameMode::IsEnemyAIActive() const
{
	return (FlowState == EFlowState::Playing || FlowState == EFlowState::Paused) && !bDebugEnemyAIDisabled;
}

void ADemoGameMode::SetAllEnemyAIEnabled() const
{
	for (TActorIterator<AEnemyMaid> Enemy(GetWorld()); Enemy; ++Enemy)
	{
		CastChecked<AEnemyMaidAIController>(Enemy->GetController())->SetAIEnabled(IsEnemyAIActive());
	}
}

void ADemoGameMode::HandleWaveCompleted()
{
	// the last enemy can despawn during the lose camera
	if (FlowState == EFlowState::Playing)
	{
		SetFlowState(EFlowState::Winning);
		PlayerMaid->StartWinSequenceCameraTransition();
	}
}

void ADemoGameMode::HandleEnemySpawned(AEnemyMaid* SpawnedEnemy)
{
	CastChecked<AEnemyMaidAIController>(SpawnedEnemy->GetController())->SetAIEnabled(IsEnemyAIActive());
}

void ADemoGameMode::HandlePlayerHealthDepleted(UHealthComponent* HealthComponent, AActor* DamageCauser)
{
	if (FlowState == EFlowState::Playing)
	{
		SetFlowState(EFlowState::Losing);
		WaveSpawner->StopWave();
		PlayerMaid->StartLoseSequenceCameraTransition();
	}
}
