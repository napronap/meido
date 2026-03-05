#include "Spawners/EnemyWaveSpawner.h"
#include "ActorComponents/HealthComponent.h"
#include "Characters/EnemyMaid.h"
#include "Engine/World.h"
#include "TimerManager.h"

AEnemyWaveSpawner::AEnemyWaveSpawner()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AEnemyWaveSpawner::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoStart)
	{
		StartWave();
	}
}

void AEnemyWaveSpawner::StartWave()
{
	if (bWaveActive || bWaveCompleted)
	{
		return;
	}

	if (TotalEnemiesToSpawn <= 0)
	{
		bWaveCompleted = true;
		OnWaveCompleted.Broadcast();
		return;
	}

	bWaveActive = true;

	if (InitialDelay <= 0.f)
	{
		TrySpawn();
	}

	if (bWaveActive && !bWaveCompleted)
	{
		const float FirstDelay = InitialDelay > 0.f ? InitialDelay : SpawnInterval;
		GetWorldTimerManager().SetTimer(
			SpawnTimerHandle,
			this,
			&AEnemyWaveSpawner::TrySpawn,
			SpawnInterval,
			true,
			FirstDelay
		);
	}
}

void AEnemyWaveSpawner::StopWave()
{
	bWaveActive = false;
	GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
}

void AEnemyWaveSpawner::ResetWave()
{
	StopWave();

	for (const TWeakObjectPtr<AEnemyMaid>& EnemyRef : AliveEnemies)
	{
		if (AEnemyMaid* Enemy = EnemyRef.Get())
		{
			Enemy->Destroy();
		}
	}

	AliveEnemies.Reset();
	SpawnedCount = 0;
	AliveCount = 0;
	bWaveCompleted = false;
}

void AEnemyWaveSpawner::TrySpawn()
{
	if (!bWaveActive || bWaveCompleted)
	{
		return;
	}

	CleanupAliveEnemies();
	EvaluateWaveCompletion();
	if (!CanSpawnMore())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World || !EnemyClass)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = SpawnCollisionHandling;
	SpawnParams.Owner = this;

	AEnemyMaid* SpawnedEnemy = World->SpawnActor<AEnemyMaid>(
		EnemyClass,
		GetActorTransform(),
		SpawnParams
	);

	if (!SpawnedEnemy)
	{
		return;
	}

	++SpawnedCount;
	++AliveCount;
	AliveEnemies.Add(SpawnedEnemy);
	SpawnedEnemy->OnDestroyed.AddDynamic(this, &AEnemyWaveSpawner::HandleEnemyDestroyed);

	OnEnemySpawned.Broadcast(SpawnedEnemy);
}

void AEnemyWaveSpawner::CleanupAliveEnemies()
{
	int32 RemovedCount = 0;
	AliveEnemies.RemoveAll(
		[&RemovedCount](const TWeakObjectPtr<AEnemyMaid>& EnemyRef)
		{
			const AEnemyMaid* Enemy = EnemyRef.Get();
			const bool bShouldRemove = (Enemy == nullptr);
			if (bShouldRemove)
			{
				++RemovedCount;
			}
			return bShouldRemove;
		}
	);

	if (RemovedCount > 0)
	{
		AliveCount = FMath::Max(0, AliveCount - RemovedCount);
	}
}

void AEnemyWaveSpawner::RemoveAliveEnemy(AEnemyMaid* Enemy)
{
	if (!Enemy)
	{
		return;
	}

	const int32 RemovedCount = AliveEnemies.RemoveAll(
		[Enemy](const TWeakObjectPtr<AEnemyMaid>& EnemyRef)
		{
			return EnemyRef.Get() == Enemy;
		}
	);

	if (RemovedCount > 0)
	{
		AliveCount = FMath::Max(0, AliveCount - RemovedCount);
	}
}

void AEnemyWaveSpawner::EvaluateWaveCompletion()
{
	if (bWaveCompleted)
	{
		return;
	}

	CleanupAliveEnemies();
	const bool bFinishedSpawningAll = SpawnedCount >= TotalEnemiesToSpawn;
	const bool bNoEnemiesAlive = GetAliveCount() <= 0;
	if (bFinishedSpawningAll && bNoEnemiesAlive)
	{
		bWaveCompleted = true;
		bWaveActive = false;
		GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
		OnWaveCompleted.Broadcast();
	}
}

bool AEnemyWaveSpawner::CanSpawnMore() const
{
	if (!EnemyClass || bWaveCompleted)
	{
		return false;
	}

	if (SpawnedCount >= TotalEnemiesToSpawn)
	{
		return false;
	}

	return AliveCount < MaxAliveAtOnce;
}

void AEnemyWaveSpawner::HandleEnemyDestroyed(AActor* DestroyedActor)
{
	RemoveAliveEnemy(Cast<AEnemyMaid>(DestroyedActor));
	CleanupAliveEnemies();
	EvaluateWaveCompletion();
	// We count enemies as "gone" only when they are actually destroyed/despawned
	// (e.g., after death lifespan), so wave completion waits for visual cleanup
	// this is so after killing the last enemy the win animation doesn't play instantly
}
