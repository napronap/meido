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

	if (!MeshComp)
	{
		return;
	}

	AActor* Owner = MeshComp->GetOwner();
	if (!Owner)
	{
		return;
	}

	if (UMeiDouComponent* MeiDouComponent = Owner->FindComponentByClass<UMeiDouComponent>())
	{
		MeiDouComponent->HandleAnimEvent(EventKey);
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

