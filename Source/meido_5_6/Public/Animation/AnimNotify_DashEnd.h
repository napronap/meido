// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_DashEnd.generated.h"

/** Optional: end dash state when the anim clip finishes (else DashDuration safety timer). */
UCLASS(meta = (DisplayName = "Dash End"))
class MEIDO_5_6_API UAnimNotify_DashEnd : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference
	) override;
};
