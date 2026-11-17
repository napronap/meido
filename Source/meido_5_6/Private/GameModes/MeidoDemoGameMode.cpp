// Fill out your copyright notice in the Description page of Project Settings.

#include "GameModes/MeidoDemoGameMode.h"

#include "ActorComponents/HealthComponent.h"
#include "ActorComponents/MeiDouComponent.h"
#include "Characters/DemoPlayerMaidCharacter.h"
#include "Characters/EnemyMaid.h"
#include "Characters/MaidCharacter.h"
#include "EngineUtils.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameModes/MeidoDemoSessionSubsystem.h"
#include "IA/CombatDirectorSubsystem.h"
#include "IA/EnemyMaidAIController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "PlayerControllers/DemoMaidPlayerController.h"
#include "Spawners/EnemyWaveSpawner.h"

AMeidoDemoGameMode::AMeidoDemoGameMode()
{
	PlayerControllerClass = ADemoMaidPlayerController::StaticClass();
	WaveSpawnerClass = AEnemyWaveSpawner::StaticClass();
	FMeidoTutorialSlide MovementSlide;
	MovementSlide.Title = NSLOCTEXT("MeidoDemo", "MovementTitle", "Move and fight");
	MovementSlide.Body = NSLOCTEXT("MeidoDemo", "MovementBody", "Move, look, dodge and attack. Press attack again during the combo window to continue the chain.");
	TutorialSlides.Add(MovementSlide);
	FMeidoTutorialSlide LockSlide;
	LockSlide.Title = NSLOCTEXT("MeidoDemo", "LockTitle", "Lock on");
	LockSlide.Body = NSLOCTEXT("MeidoDemo", "LockBody", "Lock onto an enemy to keep it in view. Switch targets or release the lock to move freely.");
	TutorialSlides.Add(LockSlide);
	FMeidoTutorialSlide PoseSlide;
	PoseSlide.Title = NSLOCTEXT("MeidoDemo", "PoseTitle", "MeiDou");
	PoseSlide.Body = NSLOCTEXT("MeidoDemo", "PoseBody", "Combine three poses: Moe, Kyun and Nyan. Moe-Moe-Kyun creates omurice; Moe-Kyun-Nyan fires a beam. An invalid recipe fails. Watch the pose queue.");
	TutorialSlides.Add(PoseSlide);
	FMeidoTutorialSlide ObjectiveSlide;
	ObjectiveSlide.Title = NSLOCTEXT("MeidoDemo", "ObjectiveTitle", "Clear the arena");
	ObjectiveSlide.Body = NSLOCTEXT("MeidoDemo", "ObjectiveBody", "Defeat all seven enemies. Watch your health. You can pause, resume or restart the run at any time during the fight.");
	TutorialSlides.Add(ObjectiveSlide);
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
	EnsureSpawner();
	NotifyPlayerPawnReady(GetPlayerPawn());
}

bool AMeidoDemoGameMode::EnsureSpawner()
{
	UWorld* DemoWorld = GetWorld();
	if (!DemoWorld)
	{
		return false;
	}
	if (IsValid(BoundWaveSpawner) && bWaveConfigured)
	{
		return true;
	}
	if (bAutoFindWaveSpawner && !IsValid(BoundWaveSpawner))
	{
		for (TActorIterator<AEnemyWaveSpawner> It(DemoWorld); It; ++It)
		{
			BoundWaveSpawner = *It;
			break;
		}
	}
	if (!IsValid(BoundWaveSpawner))
	{
		FTransform ResolvedSpawnTransform = WaveSpawnTransform;
		if (!WaveSpawnPointTag.IsNone())
		{
			for (TActorIterator<AActor> AnchorIt(DemoWorld); AnchorIt; ++AnchorIt)
			{
				if (AnchorIt->ActorHasTag(WaveSpawnPointTag))
				{
					ResolvedSpawnTransform = AnchorIt->GetActorTransform();
					break;
				}
			}
		}
		const TSubclassOf<AEnemyWaveSpawner> ResolvedSpawnerClass = WaveSpawnerClass
			? WaveSpawnerClass : TSubclassOf<AEnemyWaveSpawner>(AEnemyWaveSpawner::StaticClass());
		BoundWaveSpawner = DemoWorld->SpawnActorDeferred<AEnemyWaveSpawner>(
			ResolvedSpawnerClass, ResolvedSpawnTransform, this, nullptr,
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		if (BoundWaveSpawner)
		{
			BoundWaveSpawner->ConfigureForDemo(WaveConfig);
			UGameplayStatics::FinishSpawningActor(BoundWaveSpawner, ResolvedSpawnTransform);
		}
	}
	if (IsValid(BoundWaveSpawner))
	{
		bWaveConfigured = BoundWaveSpawner->ConfigureForDemo(WaveConfig);
		BoundWaveSpawner->OnEnemySpawned.RemoveDynamic(this, &AMeidoDemoGameMode::HandleEnemySpawned);
		BoundWaveSpawner->OnEnemySpawned.AddDynamic(this, &AMeidoDemoGameMode::HandleEnemySpawned);
		BoundWaveSpawner->OnWaveCompleted.RemoveDynamic(this, &AMeidoDemoGameMode::HandleWaveCompleted);
		BoundWaveSpawner->OnWaveCompleted.AddDynamic(this, &AMeidoDemoGameMode::HandleWaveCompleted);
		BoundWaveSpawner->OnWaveProgressChanged.RemoveDynamic(this, &AMeidoDemoGameMode::HandleWaveProgressChanged);
		BoundWaveSpawner->OnWaveProgressChanged.AddDynamic(this, &AMeidoDemoGameMode::HandleWaveProgressChanged);
		HandleWaveProgressChanged(GetRemainingEnemies(), GetTotalEnemies());
	}
	if (UCombatDirectorSubsystem* DemoDirector = DemoWorld->GetSubsystem<UCombatDirectorSubsystem>())
	{
		DemoDirector->SetMaxSimultaneousAttackers(MaxSimultaneousDemoAttackers);
	}
	if (!bWaveConfigured)
	{
		UE_LOG(LogTemp, Error, TEXT("Demo setup incomplete: assign WaveConfig.EnemyClass on the Demo GameMode and check WaveSpawnerClass."));
	}
	return bWaveConfigured;
}

void AMeidoDemoGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	bResettingRun = true;
	if (IsValid(BoundWaveSpawner))
	{
		BoundWaveSpawner->StopWave();
		BoundWaveSpawner->OnEnemySpawned.RemoveDynamic(this, &AMeidoDemoGameMode::HandleEnemySpawned);
		BoundWaveSpawner->OnWaveCompleted.RemoveDynamic(this, &AMeidoDemoGameMode::HandleWaveCompleted);
		BoundWaveSpawner->OnWaveProgressChanged.RemoveDynamic(this, &AMeidoDemoGameMode::HandleWaveProgressChanged);
	}
	BindPlayerHealthFromPawn(nullptr);
	Super::EndPlay(EndPlayReason);
}

int32 AMeidoDemoGameMode::GetRemainingEnemies() const
{
	return IsValid(BoundWaveSpawner) ? BoundWaveSpawner->GetRemainingEnemyCount() : FMath::Max(1, WaveConfig.TotalEnemies);
}

int32 AMeidoDemoGameMode::GetTotalEnemies() const
{
	return IsValid(BoundWaveSpawner) ? BoundWaveSpawner->GetTotalEnemyCount() : FMath::Max(1, WaveConfig.TotalEnemies);
}

void AMeidoDemoGameMode::HandleWaveProgressChanged(const int32 InRemainingEnemies, const int32 InTotalEnemies)
{
	if (!bResettingRun)
	{
		OnWaveProgressChanged.Broadcast(InRemainingEnemies, InTotalEnemies);
	}
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
	if (FlowState == NewState && bFlowPresentationInitialized)
	{
		return;
	}

	FlowState = NewState;
	bFlowPresentationInitialized = true;
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
	if (!EnsureSpawner())
	{
		EnterMainMenu();
		return;
	}
	bWaitingForWinSequence = false;
	bWaitingForLoseSequence = false;
	bWaitingForMenuCameraTransition = false;
	SetFlowState(EFlowState::Playing);

	// Re-bind health after restart (pawn may have been reset).
	BindPlayerHealthFromPawn(GetPlayerPawn());
	if (!bRunHasStarted)
	{
		bRunHasStarted = true;
		BoundWaveSpawner->StartWave();
	}

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
	bResettingRun = true;
	bRunHasStarted = false;
	if (ADemoMaidPlayerController* PC = GetDemoPC())
	{
		PC->PrepareLocalRestartPresentation();
	}

	UGameplayStatics::SetGlobalTimeDilation(this, 1.0f);
	if (IsValid(BoundWaveSpawner))
	{
		BoundWaveSpawner->ResetWave();
	}

	if (AMaidCharacter* Maid = GetPlayerMaid())
	{
		Maid->CustomTimeDilation = 1.0f;
		Maid->ResetForFlowRestart();
		Maid->ConsumeMovementInputVector();
		if (RestartPawn.Get() == Maid)
		{
			Maid->SetActorTransform(RestartPawnTransform, false, nullptr, ETeleportType::TeleportPhysics);
		}
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
	bResettingRun = false;
	HandleWaveProgressChanged(GetRemainingEnemies(), GetTotalEnemies());
}

void AMeidoDemoGameMode::RequestStartFromMenu()
{
	if (FlowState == EFlowState::Win || FlowState == EFlowState::Lose)
	{
		RequestRestartRun();
		return;
	}
	if (bResettingRun || FlowState != EFlowState::MainMenu || bWaitingForMenuCameraTransition || !GetPlayerMaid() || !EnsureSpawner())
	{
		return;
	}
	if (ShouldShowTutorial())
	{
		TutorialSlideIndex = 0;
		SetFlowState(EFlowState::Tutorial);
		OnTutorialSlideChanged.Broadcast(TutorialSlideIndex, TutorialSlides.Num());
		return;
	}
	BeginGameplayTransition();
}

void AMeidoDemoGameMode::BeginGameplayTransition()
{
	// Set before calling the legacy camera; it may finish synchronously.
	bWaitingForMenuCameraTransition = true;
	if (ADemoMaidPlayerController* PC = GetDemoPC())
	{
		PC->PrepareLocalRestartPresentation();
		PC->ApplyRestartInterimPresentation();
	}
	if (ADemoPlayerMaidCharacter* DemoPlayer = Cast<ADemoPlayerMaidCharacter>(GetPlayerPawn()))
	{
		DemoPlayer->StartMenuCameraTransitionToGameplay();
		if (DemoPlayer->IsMenuCameraTransitionActive())
		{
			return;
		}
	}

	NotifyMenuCameraFinished();
}

void AMeidoDemoGameMode::NotifyMenuCameraFinished()
{
	if (!bWaitingForMenuCameraTransition || (FlowState != EFlowState::MainMenu && FlowState != EFlowState::Tutorial))
	{
		return;
	}

	bWaitingForMenuCameraTransition = false;
	EnterPlaying();
}

void AMeidoDemoGameMode::NotifyPlayerPawnReady(APawn* Pawn)
{
	BindPlayerHealthFromPawn(Pawn);
	if (Pawn && RestartPawn.Get() != Pawn)
	{
		RestartPawn = Pawn;
		RestartPawnTransform = Pawn->GetActorTransform();
	}
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
	if (FlowState != EFlowState::Paused || bWaitingForWinSequence || bWaitingForLoseSequence || bResettingRun)
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
	if (bResettingRun || bWaitingForMenuCameraTransition || bWaitingForWinSequence || bWaitingForLoseSequence)
	{
		return;
	}
	if (FlowState != EFlowState::Paused && FlowState != EFlowState::Win && FlowState != EFlowState::Lose)
	{
		return;
	}
	if (!EnsureSpawner())
	{
		return;
	}
	PrepareRestartFromResult();
	BeginGameplayTransition();
}

bool AMeidoDemoGameMode::ShouldShowTutorial() const
{
	if (!bShowTutorialOnFirstRun || TutorialSlides.IsEmpty())
	{
		return false;
	}
	const UGameInstance* DemoInstance = GetGameInstance();
	const UMeidoDemoSessionSubsystem* DemoSession = DemoInstance
		? DemoInstance->GetSubsystem<UMeidoDemoSessionSubsystem>() : nullptr;
	if (DemoSession && DemoSession->HasCompletedTutorial())
	{
		return false;
	}
	const ADemoMaidPlayerController* TutorialPC = GetDemoPC();
	if (!TutorialPC || !TutorialPC->CanPresentTutorial())
	{
		UE_LOG(LogTemp, Warning, TEXT("Demo tutorial skipped: assign TutorialWidgetClass on the demo controller."));
		return false;
	}
	return true;
}

bool AMeidoDemoGameMode::GetCurrentTutorialSlide(FMeidoTutorialSlide& OutSlide) const
{
	if (FlowState != EFlowState::Tutorial || !TutorialSlides.IsValidIndex(TutorialSlideIndex))
	{
		OutSlide = FMeidoTutorialSlide();
		return false;
	}
	OutSlide = TutorialSlides[TutorialSlideIndex];
	return true;
}

void AMeidoDemoGameMode::RequestNextTutorialSlide()
{
	if (FlowState != EFlowState::Tutorial || bWaitingForMenuCameraTransition)
	{
		return;
	}
	if (TutorialSlides.IsValidIndex(TutorialSlideIndex + 1))
	{
		++TutorialSlideIndex;
		OnTutorialSlideChanged.Broadcast(TutorialSlideIndex, TutorialSlides.Num());
		return;
	}
	if (UGameInstance* DemoInstance = GetGameInstance())
	{
		if (UMeidoDemoSessionSubsystem* DemoSession = DemoInstance->GetSubsystem<UMeidoDemoSessionSubsystem>())
		{
			DemoSession->MarkTutorialCompleted();
		}
	}
	BeginGameplayTransition();
}

void AMeidoDemoGameMode::RequestPreviousTutorialSlide()
{
	if (FlowState == EFlowState::Tutorial && !bWaitingForMenuCameraTransition && TutorialSlideIndex > 0)
	{
		--TutorialSlideIndex;
		OnTutorialSlideChanged.Broadcast(TutorialSlideIndex, TutorialSlides.Num());
	}
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

	bWaitingForWinSequence = true;
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
	bWaitingForLoseSequence = true;

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
	if (bResettingRun || bWaitingForLoseSequence || FlowState == EFlowState::Lose)
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
	if (bResettingRun || bWaitingForLoseSequence || FlowState == EFlowState::Lose)
	{
		return;
	}

	if (FlowState != EFlowState::Playing && FlowState != EFlowState::Paused)
	{
		return;
	}

	StartLoseSequence();
}
