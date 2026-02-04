// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Types/MeiDouTypes.h"
#include "MeiDouPoseDataAsset.generated.h"

/**
 * A data asset representing each individual pose's data related to animations
 * this should live in the character since it's the one driving animations
 * for example, different characters can have different assets for each pose
 */
UCLASS()
class MEIDO_5_6_API UMeiDouPoseDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EMeiDouInput Input;
	
	/*
	 * if a pose is played twice in the same combo string, it should play the mirrored version
	 * the third time it should play the normal version again
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UAnimMontage* Montage;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UAnimMontage* MirroredMontage;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float PlayRate = 1.5f;
};
