// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotifyState_MeiDouPoseActive.h"
#include "ActorComponents/MeiDouComponent.h"
#include "Components/SkeletalMeshComponent.h"

void UAnimNotifyState_MeiDouPoseActive::NotifyBegin(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float TotalDuration,
	const FAnimNotifyEventReference& EventReference
)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (!MeshComp)
	{
		return;
	}

	if (AActor* Owner = MeshComp->GetOwner())
	{
		if (UMeiDouComponent* MeiDouComponent = Owner->FindComponentByClass<UMeiDouComponent>())
		{
			MeiDouComponent->OnMeiDouActionWindowBegin();
		}
	}
}

void UAnimNotifyState_MeiDouPoseActive::NotifyEnd(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference
)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (!MeshComp)
	{
		return;
	}

	if (AActor* Owner = MeshComp->GetOwner())
	{
		if (UMeiDouComponent* MeiDouComponent = Owner->FindComponentByClass<UMeiDouComponent>())
		{
			MeiDouComponent->OnMeiDouActionWindowEnd();
		}
	}
}

FString UAnimNotifyState_MeiDouPoseActive::GetNotifyName_Implementation() const
{
	return TEXT("MeiDou Pose Active");
}

