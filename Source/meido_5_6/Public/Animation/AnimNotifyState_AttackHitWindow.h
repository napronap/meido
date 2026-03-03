// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotifyState_AttackHitWindow.generated.h"

/**
 * 
 */
UCLASS()
class MEIDO_5_6_API UAnimNotifyState_AttackHitWindow : public UAnimNotifyState
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

	// Legacy single-socket entry. Used when HitSocketNames is empty.
	UPROPERTY(EditAnywhere, Category="Hit")
	FName HitSocketName = "Attack_Hand_R";

	// Optional multi-socket setup for a wider/multi-point hit window.
	UPROPERTY(EditAnywhere, Category="Hit")
	TArray<FName> HitSocketNames;
};
