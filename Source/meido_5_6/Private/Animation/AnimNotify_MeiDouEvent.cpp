// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotify_MeiDouEvent.h"
#include "ActorComponents/MeiDouComponent.h"
#include "Components/SkeletalMeshComponent.h"

void UAnimNotify_MeiDouEvent::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference
)
{
	Super::Notify(MeshComp, Animation, EventReference);

	// preview mesh in the montage editor has no MeiDou component
	if (UMeiDouComponent* MeiDouComponent = MeshComp->GetOwner()->FindComponentByClass<UMeiDouComponent>())
	{
		MeiDouComponent->HandleAnimEvent(EventKey, Animation);
	}
}

FString UAnimNotify_MeiDouEvent::GetNotifyName_Implementation() const
{
	if (const UEnum* Enum = StaticEnum<EMeiDouAnimEvent>())
	{
		return FString::Printf(TEXT("MeiDou: %s"), *Enum->GetNameStringByValue(static_cast<int64>(EventKey)));
	}

	return TEXT("MeiDou Event");
}

