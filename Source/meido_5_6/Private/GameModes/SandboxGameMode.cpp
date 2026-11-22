#include "GameModes/SandboxGameMode.h"
#include "Characters/EnemyMaid.h"
#include "EngineUtils.h"
#include "IA/EnemyMaidAIController.h"
#include "Spawners/EnemyWaveSpawner.h"

void ASandboxGameMode::StartPlay()
{
	Super::StartPlay();

	// the enemies placed in the map, then the ones the spawners bring
	SetAllEnemyAIEnabled();

	for (TActorIterator<AEnemyWaveSpawner> Spawner(GetWorld()); Spawner; ++Spawner)
	{
		Spawner->OnEnemySpawned.AddDynamic(this, &ASandboxGameMode::HandleEnemySpawned);
		Spawner->StartWave();
	}
}

void ASandboxGameMode::ToggleDebugEnemyAI()
{
	bDebugEnemyAIDisabled = !bDebugEnemyAIDisabled;
	SetAllEnemyAIEnabled();
}

void ASandboxGameMode::SetAllEnemyAIEnabled() const
{
	for (TActorIterator<AEnemyMaid> Enemy(GetWorld()); Enemy; ++Enemy)
	{
		CastChecked<AEnemyMaidAIController>(Enemy->GetController())->SetAIEnabled(!bDebugEnemyAIDisabled);
	}
}

void ASandboxGameMode::HandleEnemySpawned(AEnemyMaid* SpawnedEnemy)
{
	CastChecked<AEnemyMaidAIController>(SpawnedEnemy->GetController())->SetAIEnabled(!bDebugEnemyAIDisabled);
}
