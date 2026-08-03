// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotifyState_DashIFrames.generated.h"

/**
 * Place on dash anim / blend-space samples so i-frames match art (CP2.1).
 * Begin → invuln on; End → invuln off. Prefer this over legacy timed i-frames on dash start.
 */
UCLASS(meta = (DisplayName = "Dash I-Frames"))
class MEIDO_5_6_API UAnimNotifyState_DashIFrames : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		float TotalDuration,
		const FAnimNotifyEventReference& EventReference
	) override;

	virtual void NotifyEnd(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference
	) override;
};
