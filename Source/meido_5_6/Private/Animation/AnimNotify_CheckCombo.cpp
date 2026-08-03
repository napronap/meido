// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotify_CheckCombo.h"
#include "ActorComponents/AttackComponent.h"

void UAnimNotify_CheckCombo::Notify(
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

	if (UAttackComponent* Attack = Owner->FindComponentByClass<UAttackComponent>())
	{
		Attack->NotifyCheckCombo();
	}
}
