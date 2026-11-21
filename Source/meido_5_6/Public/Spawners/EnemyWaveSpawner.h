#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemyWaveSpawner.generated.h"

class AEnemyMaid;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEnemyWaveCompleted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyWaveSpawnedEnemy, AEnemyMaid*, SpawnedEnemy);

// Spawns TotalEnemiesToSpawn enemies, keeping at most MaxAliveAtOnce alive
// The wave completes once every enemy was spawned and destroyed (after its death lifespan, not on death)
UCLASS()
class MEIDO_5_6_API AEnemyWaveSpawner : public AActor
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Spawner")
	void StartWave();

	UFUNCTION(BlueprintCallable, Category="Spawner")
	void StopWave();

	// stops and restarts the count, ready for StartWave again (alive enemies are cleared by the flow)
	UFUNCTION(BlueprintCallable, Category="Spawner")
	void ResetWave();

	UFUNCTION(BlueprintPure, Category="Spawner")
	bool IsWaveCompleted() const { return SpawnedCount >= TotalEnemiesToSpawn && AliveEnemies.IsEmpty(); }

	UPROPERTY(BlueprintAssignable, Category="Spawner")
	FOnEnemyWaveCompleted OnWaveCompleted;

	UPROPERTY(BlueprintAssignable, Category="Spawner")
	FOnEnemyWaveSpawnedEnemy OnEnemySpawned;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spawner")
	TSubclassOf<AEnemyMaid> EnemyClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spawner", meta=(ClampMin="1", UIMin="1"))
	int32 TotalEnemiesToSpawn = 6;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spawner", meta=(ClampMin="1", UIMin="1"))
	int32 MaxAliveAtOnce = 2;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spawner", meta=(ClampMin="0.01", UIMin="0.01"))
	float SpawnInterval = 1.25f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spawner", meta=(ClampMin="0.0", UIMin="0.0"))
	float InitialDelay = 0.0f;

private:
	// tries a spawn every SpawnInterval until the whole wave was spawned
	void TrySpawn();

	UFUNCTION()
	void HandleEnemyDestroyed(AActor* DestroyedActor);

	FTimerHandle SpawnTimerHandle;

	UPROPERTY(Transient)
	TArray<TObjectPtr<AEnemyMaid>> AliveEnemies;

	int32 SpawnedCount = 0;
};
