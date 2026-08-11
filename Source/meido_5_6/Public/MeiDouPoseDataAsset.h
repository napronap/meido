// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Types/MeiDouTypes.h"
#include "MeiDouPoseDataAsset.generated.h"

class USoundBase;

/**
 * Per-pose data for one MeiDou input (montage, rate, SFX).
 * New poses = new assets mapped on the caster; no hard-coded Moe/Kyun/Nyan audio table.
 */
UCLASS()
class MEIDO_5_6_API UMeiDouPoseDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MeiDou|Pose")
	EMeiDouInput Input = EMeiDouInput::None;

	/**
	 * If a pose is played twice in the same combo string, mirror via anim flag
	 * (2nd, 4th… — see UMeiDouComponent::ShouldMirrorPose).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MeiDou|Pose")
	TObjectPtr<UAnimMontage> Montage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MeiDou|Pose", meta = (ClampMin = "0.01"))
	float PlayRate = 1.5f;

	/** Played when this pose starts (CP3.6). Injected from this asset via FindPoseData. Null = silence. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MeiDou|SFX")
	TObjectPtr<USoundBase> PoseSfx;

	/** Optional SFX when mirrored; if null, uses PoseSfx. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MeiDou|SFX")
	TObjectPtr<USoundBase> MirroredPoseSfx;
};
