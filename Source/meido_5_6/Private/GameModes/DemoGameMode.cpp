#include "GameModes/DemoGameMode.h"
#include "ActorComponents/HealthComponent.h"
#include "Animation/AnimMontage.h"
#include "Characters/EnemyMaid.h"
#include "Characters/PlayerMaidCharacter.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "IA/EnemyMaidAIController.h"
#include "Kismet/GameplayStatics.h"
#include "Spawners/EnemyWaveSpawner.h"
#include "TimerManager.h"

void ADemoGameMode::StartPlay()
{
	Super::StartPlay();

	PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	CameraManager = CastChecked<ADemoCameraManager>(PlayerController->PlayerCameraManager);

	PlayerMaid = CastChecked<APlayerMaidCharacter>(PlayerController->GetPawn());
	PlayerMaid->GetHealthComponent()->OnHealthDepleted.AddDynamic(this, &ADemoGameMode::HandlePlayerHealthDepleted);

	// the level has exactly one spawner
	WaveSpawner = *TActorIterator<AEnemyWaveSpawner>(GetWorld());
	WaveSpawner->OnEnemySpawned.AddDynamic(this, &ADemoGameMode::HandleEnemySpawned);
	WaveSpawner->OnWaveCompleted.AddDynamic(this, &ADemoGameMode::HandleWaveCompleted);

	SetFlowState(EFlowState::MainMenu);
	CameraManager->SnapTo(MenuShot);
	PlayerMaid->PlayAnimMontage(MenuPoseMontage, 1.f, MenuPoseSection);
	PlayerMaid->SetExpression(EMaidExpression::Wink);
}

void ADemoGameMode::StartGame()
{
	// called from the main menu, or from Win/Lose/Paused to retry
	const bool bRetry = FlowState != EFlowState::MainMenu;
	if (bRetry)
	{
		PlayerMaid->ResetForFlowRestart();
		ClearWave();
	}

	StartTransition(EFlowState::Starting, GameplayBlendTime);
	CameraManager->BlendToPawn(GameplayPitch, GameplayBlendTime);
}

void ADemoGameMode::StartPlaying()
{
	SetFlowState(EFlowState::Playing);
	WaveSpawner->StartWave();
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

void ADemoGameMode::StartTransition(const EFlowState TransitionState, const float BlendTime)
{
	SetFlowState(TransitionState);
	GetWorldTimerManager().SetTimer(TransitionTimerHandle, this, &ADemoGameMode::FinishTransition, BlendTime);
}

void ADemoGameMode::FinishTransition()
{
	switch (FlowState)
	{
	case EFlowState::Starting:
		PlayerMaid->SetExpression(EMaidExpression::Neutral);
		if (!bTutorialShown)
		{
			bTutorialShown = true;
			SetFlowState(EFlowState::Tutorial);
			return;
		}
		StartPlaying();
		return;

	case EFlowState::Winning:
		SetFlowState(EFlowState::Win);
		return;

	case EFlowState::Losing:
		// the enemies still alive leave with the lose screen
		ClearWave();
		SetFlowState(EFlowState::Lose);
		return;

	default:
		return;
	}
}

void ADemoGameMode::ClearWave() const
{
	WaveSpawner->ResetWave();
	for (TActorIterator<AEnemyMaid> Enemy(GetWorld()); Enemy; ++Enemy)
	{
		Enemy->Destroy();
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

	// pause if the player pauses, play if the player is playing, and 
	// inverse playing if we are pausing
	const bool bPaused = FlowState == EFlowState::Paused;
	PlayerController->SetPause(bPaused);

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
		// dead: Die already turned its AI off for good
		// being destroyed: the last one completes the wave from its OnDestroyed, when the iterator still
		// returns it but APawn::Destroyed already detached it from its controller
		if (Enemy->IsDead() || Enemy->IsActorBeingDestroyed())
		{
			continue;
		}

		CastChecked<AEnemyMaidAIController>(Enemy->GetController())->SetAIEnabled(IsEnemyAIActive());
	}
}

void ADemoGameMode::HandleWaveCompleted()
{
	// the last enemy can despawn during the lose camera
	if (FlowState == EFlowState::Playing)
	{
		StartTransition(EFlowState::Winning, WinShot.BlendTime);
		CameraManager->BlendTo(WinShot);
		PlayerMaid->PlayAnimMontage(WinMontage);
		PlayerMaid->SetExpression(EMaidExpression::Happy);
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
		WaveSpawner->StopWave();
		StartTransition(EFlowState::Losing, LoseShot.BlendTime);
		CameraManager->BlendTo(LoseShot);
	}
}
