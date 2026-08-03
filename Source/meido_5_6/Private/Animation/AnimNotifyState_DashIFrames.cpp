// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotifyState_DashIFrames.h"
#include "Characters/MaidCharacter.h"
#include "Components/SkeletalMeshComponent.h"

void UAnimNotifyState_DashIFrames::NotifyBegin(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const float TotalDuration,
	const FAnimNotifyEventReference& EventReference
)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr;
	if (AMaidCharacter* Maid = Cast<AMaidCharacter>(Owner))
	{
		// Only player needs dash i-frames (same policy as legacy GrantIFrames on start).
		if (Maid->IsPlayerControlled())
		{
			Maid->SetIFrameOverrideActive(true);
		}
	}
}

void UAnimNotifyState_DashIFrames::NotifyEnd(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference
)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr;
	if (AMaidCharacter* Maid = Cast<AMaidCharacter>(Owner))
	{
		Maid->SetIFrameOverrideActive(false);
	}
}
