#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemyWaveSpawner.generated.h"

class AEnemyMaid;
class UHealthComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEnemyWaveCompleted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyWaveSpawnedEnemy, AEnemyMaid*, SpawnedEnemy);

UCLASS()
class MEIDO_5_6_API AEnemyWaveSpawner : public AActor
{
	GENERATED_BODY()

public:
	AEnemyWaveSpawner();

	UFUNCTION(BlueprintCallable, Category="Spawner")
	void StartWave();

	UFUNCTION(BlueprintCallable, Category="Spawner")
	void StopWave();

	UFUNCTION(BlueprintCallable, Category="Spawner")
	void ResetWave();

	UFUNCTION(BlueprintPure, Category="Spawner")
	int32 GetSpawnedCount() const { return SpawnedCount; }

	UFUNCTION(BlueprintPure, Category="Spawner")
	int32 GetAliveCount() const { return AliveCount; }

	UFUNCTION(BlueprintPure, Category="Spawner")
	bool IsWaveCompleted() const { return bWaveCompleted; }

	UPROPERTY(BlueprintAssignable, Category="Spawner")
	FOnEnemyWaveCompleted OnWaveCompleted;

	UPROPERTY(BlueprintAssignable, Category="Spawner")
	FOnEnemyWaveSpawnedEnemy OnEnemySpawned;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spawner")
	TSubclassOf<AEnemyMaid> EnemyClass;

	// Total enemies in this wave. If all are dead, wave is completed.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spawner", meta=(ClampMin="0", UIMin="0"))
	int32 TotalEnemiesToSpawn = 6;

	// Max alive enemies at once. The spawner refills until reaching TotalEnemiesToSpawn.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spawner", meta=(ClampMin="1", UIMin="1"))
	int32 MaxAliveAtOnce = 2;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spawner", meta=(ClampMin="0.01", UIMin="0.01"))
	float SpawnInterval = 1.25f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spawner", meta=(ClampMin="0.0", UIMin="0.0"))
	float InitialDelay = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spawner")
	bool bAutoStart = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spawner")
	ESpawnActorCollisionHandlingMethod SpawnCollisionHandling = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

private:
	FTimerHandle SpawnTimerHandle;

	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<AEnemyMaid>> AliveEnemies;

	int32 SpawnedCount = 0;
	int32 AliveCount = 0;
	bool bWaveActive = false;
	bool bWaveCompleted = false;

	void TrySpawn();
	void CleanupAliveEnemies();
	void EvaluateWaveCompletion();
	bool CanSpawnMore() const;
	void RemoveAliveEnemy(AEnemyMaid* Enemy);

	UFUNCTION()
	void HandleEnemyDestroyed(AActor* DestroyedActor);

	UFUNCTION()
	void HandleEnemyHealthDepleted(UHealthComponent* HealthComponent, AActor* DamageCauser);
};
