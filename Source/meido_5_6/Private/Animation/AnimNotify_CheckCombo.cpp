// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotify_CheckCombo.h"
#include "ActorComponents/AttackComponent.h"
#include "Components/SkeletalMeshComponent.h"

void UAnimNotify_CheckCombo::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference
)
{
	Super::Notify(MeshComp, Animation, EventReference);

	// preview mesh in the montage editor has no attack component
	if (UAttackComponent* AttackComponent = MeshComp->GetOwner()->FindComponentByClass<UAttackComponent>())
	{
		AttackComponent->CheckCombo(Animation);
	}
}
