// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotify_DashEnd.h"
#include "ActorComponents/DashComponent.h"
#include "Components/SkeletalMeshComponent.h"

void UAnimNotify_DashEnd::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference
)
{
	Super::Notify(MeshComp, Animation, EventReference);

	AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr;
	if (!Owner)
	{
		return;
	}

	if (UDashComponent* Dash = Owner->FindComponentByClass<UDashComponent>())
	{
		Dash->NotifyDashAnimationEnded();
	}
}
