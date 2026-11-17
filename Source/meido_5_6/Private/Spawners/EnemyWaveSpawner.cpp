#include "Spawners/EnemyWaveSpawner.h"
#include "ActorComponents/HealthComponent.h"
#include "Characters/EnemyMaid.h"
#include "Components/SceneComponent.h"
#include "GameModes/MeidoDemoGameMode.h"
#include "Engine/World.h"
#include "TimerManager.h"

AEnemyWaveSpawner::AEnemyWaveSpawner()
{
	PrimaryActorTick.bCanEverTick = false;
	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("SpawnRoot")));
#if WITH_EDITORONLY_DATA
	// Show Unreal's default actor marker in the level editor.
	GetRootComponent()->bVisualizeComponent = true;
#endif
}

void AEnemyWaveSpawner::BeginPlay()
{
	Super::BeginPlay();

	// A placed spawner may BeginPlay before the GM binds it. The demo owns start timing.
	const bool bDemoWorld = GetWorld() && GetWorld()->GetAuthGameMode<AMeidoDemoGameMode>();
	if (bAutoStart && !bManagedByDemoFlow && !bDemoWorld)
	{
		StartWave();
	}
}

void AEnemyWaveSpawner::StartWave()
{
	if (bResettingWave || bWaveActive || bWaveCompleted)
	{
		return;
	}

	if (TotalEnemiesToSpawn <= 0)
	{
		bWaveCompleted = true;
		OnWaveCompleted.Broadcast();
		return;
	}

	if (!EnemyClass || !GetWorld())
	{
		UE_LOG(LogTemp, Warning, TEXT("EnemyWaveSpawner: assign EnemyClass before starting the wave."));
		return;
	}
	SpawnInterval = FMath::Max(0.05f, SpawnInterval);
	MaxAliveAtOnce = FMath::Max(1, MaxAliveAtOnce);

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
	if (bResettingWave)
	{
		return;
	}
	StopWave();
	bResettingWave = true;
	// Destroy broadcasts synchronously. Detach the tracked array before touching actors.
	const TArray<TWeakObjectPtr<AEnemyMaid>> EnemiesToReset = AliveEnemies;
	AliveEnemies.Reset();
	SpawnedCount = 0;
	AliveCount = 0;
	bWaveCompleted = false;
	for (const TWeakObjectPtr<AEnemyMaid>& EnemyRef : EnemiesToReset)
	{
		if (AEnemyMaid* Enemy = EnemyRef.Get())
		{
			Enemy->OnDestroyed.RemoveDynamic(this, &AEnemyWaveSpawner::HandleEnemyDestroyed);
			if (UHealthComponent* EnemyHealth = Enemy->FindComponentByClass<UHealthComponent>())
			{
				EnemyHealth->OnHealthDepleted.RemoveDynamic(this, &AEnemyWaveSpawner::HandleEnemyHealthDepleted);
			}
			Enemy->Destroy();
		}
	}
	bResettingWave = false;
	BroadcastWaveProgress();
}

void AEnemyWaveSpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopWave();
	bResettingWave = true;
	for (const TWeakObjectPtr<AEnemyMaid>& EnemyEntry : AliveEnemies)
	{
		if (AEnemyMaid* TrackedEnemy = EnemyEntry.Get())
		{
			TrackedEnemy->OnDestroyed.RemoveDynamic(this, &AEnemyWaveSpawner::HandleEnemyDestroyed);
			if (UHealthComponent* TrackedHealth = TrackedEnemy->FindComponentByClass<UHealthComponent>())
			{
				TrackedHealth->OnHealthDepleted.RemoveDynamic(this, &AEnemyWaveSpawner::HandleEnemyHealthDepleted);
			}
		}
	}
	Super::EndPlay(EndPlayReason);
}

bool AEnemyWaveSpawner::ConfigureForDemo(const FMeidoWaveConfig& InWaveConfig)
{
	bManagedByDemoFlow = true;
	ResetWave();
	EnemyClass = InWaveConfig.EnemyClass;
	TotalEnemiesToSpawn = FMath::Max(1, InWaveConfig.TotalEnemies);
	MaxAliveAtOnce = FMath::Clamp(InWaveConfig.MaxAlive, 1, TotalEnemiesToSpawn);
	SpawnInterval = FMath::Max(0.05f, InWaveConfig.SpawnInterval);
	InitialDelay = FMath::Max(0.f, InWaveConfig.InitialDelay);
	BroadcastWaveProgress();
	return EnemyClass != nullptr;
}

int32 AEnemyWaveSpawner::GetRemainingEnemyCount() const
{
	int32 LivingTrackedEnemies = 0;
	for (const TWeakObjectPtr<AEnemyMaid>& EnemyEntry : AliveEnemies)
	{
		const AEnemyMaid* TrackedEnemy = EnemyEntry.Get();
		if (IsValid(TrackedEnemy) && !TrackedEnemy->IsDead())
		{
			++LivingTrackedEnemies;
		}
	}
	return FMath::Clamp(TotalEnemiesToSpawn - SpawnedCount + LivingTrackedEnemies, 0, TotalEnemiesToSpawn);
}

void AEnemyWaveSpawner::BroadcastWaveProgress()
{
	OnWaveProgressChanged.Broadcast(GetRemainingEnemyCount(), TotalEnemiesToSpawn);
}

void AEnemyWaveSpawner::HandleEnemyHealthDepleted(UHealthComponent* InHealthComponent, AActor* InDamageCauser)
{
	if (!bResettingWave)
	{
		// Counter updates on death; completion still waits for the corpse's existing lifespan.
		BroadcastWaveProgress();
	}
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
	if (UHealthComponent* SpawnedHealth = SpawnedEnemy->FindComponentByClass<UHealthComponent>())
	{
		SpawnedHealth->OnHealthDepleted.AddDynamic(this, &AEnemyWaveSpawner::HandleEnemyHealthDepleted);
	}

	OnEnemySpawned.Broadcast(SpawnedEnemy);
	BroadcastWaveProgress();
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
	if (bResettingWave || !bWaveActive || bWaveCompleted)
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
	if (bResettingWave)
	{
		return;
	}
	RemoveAliveEnemy(Cast<AEnemyMaid>(DestroyedActor));
	CleanupAliveEnemies();
	BroadcastWaveProgress();
	EvaluateWaveCompletion();
	// We count enemies as "gone" only when they are actually destroyed/despawned
	// (e.g., after death lifespan), so wave completion waits for visual cleanup
	// this is so after killing the last enemy the win animation doesn't play instantly
}
