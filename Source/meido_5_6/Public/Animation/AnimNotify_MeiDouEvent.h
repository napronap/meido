// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Types/MeiDouTypes.h"
#include "AnimNotify_MeiDouEvent.generated.h"

/**
 * 
 */
UCLASS()
class MEIDO_5_6_API UAnimNotify_MeiDouEvent : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference
	) override;

	virtual FString GetNotifyName_Implementation() const override;

protected:
	UPROPERTY(EditAnywhere, Category="MeiDou")
	EMeiDouAnimEvent EventKey = EMeiDouAnimEvent::EMDAE_Spawn;
};
