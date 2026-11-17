#pragma once

#include "CoreMinimal.h"
#include "DemoContentTypes.generated.h"

class AEnemyMaid;
class UTexture2D;

USTRUCT(BlueprintType)
struct FMeidoTutorialSlide
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tutorial")
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tutorial", meta = (MultiLine = true))
	FText Body;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tutorial")
	TObjectPtr<UTexture2D> Image = nullptr;
};

/** Demo rules; copied into the executor before it starts. */
USTRUCT(BlueprintType)
struct FMeidoWaveConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wave")
	TSubclassOf<AEnemyMaid> EnemyClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wave", meta = (ClampMin = "1"))
	int32 TotalEnemies = 7;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wave", meta = (ClampMin = "1"))
	int32 MaxAlive = 2;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wave", meta = (ClampMin = "0.05"))
	float SpawnInterval = 1.25f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wave", meta = (ClampMin = "0.0"))
	float InitialDelay = 0.f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMeidoTutorialSlideChanged, int32, SlideIndex, int32, SlideCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMeidoWaveProgress, int32, RemainingEnemies, int32, TotalEnemies);
