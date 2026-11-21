#include "Spawners/EnemyWaveSpawner.h"
#include "Characters/EnemyMaid.h"
#include "Engine/World.h"
#include "TimerManager.h"

void AEnemyWaveSpawner::StartWave()
{
	GetWorldTimerManager().SetTimer(
		SpawnTimerHandle,
		this,
		&AEnemyWaveSpawner::TrySpawn,
		SpawnInterval,
		true,
		InitialDelay
	);
}

void AEnemyWaveSpawner::StopWave()
{
	GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
}

void AEnemyWaveSpawner::ResetWave()
{
	StopWave();
	SpawnedCount = 0;
}

void AEnemyWaveSpawner::TrySpawn()
{
	if (AliveEnemies.Num() >= MaxAliveAtOnce)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	SpawnParams.Owner = this;

	AEnemyMaid* Enemy = GetWorld()->SpawnActor<AEnemyMaid>(EnemyClass, GetActorTransform(), SpawnParams);
	Enemy->OnDestroyed.AddDynamic(this, &AEnemyWaveSpawner::HandleEnemyDestroyed);
	AliveEnemies.Add(Enemy);
	++SpawnedCount;

	if (SpawnedCount >= TotalEnemiesToSpawn)
	{
		StopWave();
	}

	OnEnemySpawned.Broadcast(Enemy);
}

void AEnemyWaveSpawner::HandleEnemyDestroyed(AActor* DestroyedActor)
{
	AliveEnemies.Remove(CastChecked<AEnemyMaid>(DestroyedActor));

	if (IsWaveCompleted())
	{
		OnWaveCompleted.Broadcast();
	}
}
