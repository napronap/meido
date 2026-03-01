// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "MeiDouSpawnConfigurable.generated.h"

class AActor;

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UMeiDouSpawnConfigurable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class MEIDO_5_6_API IMeiDouSpawnConfigurable
{
	GENERATED_BODY()

public:
	// Called right after deferred spawn so the spawned actor can initialize target/end behavior.
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="MeiDou")
	void ConfigureMeiDouSpawn(AActor* SourceActor, AActor* EndActor, FVector EndLocation, bool bUseEndActor);

	// Called when the current MeiDou pose window ends.
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="MeiDou")
	void OnMeiDouActionEnded();
};
