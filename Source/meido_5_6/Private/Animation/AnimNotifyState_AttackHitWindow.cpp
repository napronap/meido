// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotifyState_AttackHitWindow.h"
#include "ActorComponents/AttackComponent.h"
#include "Components/SkeletalMeshComponent.h"

void UAnimNotifyState_AttackHitWindow::NotifyBegin(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float TotalDuration,
	const FAnimNotifyEventReference& EventReference
)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	// need this check for the notify to not crash when it doesn't find the attack component (preview mesh won't have it)
	if (UAttackComponent* AttackComponent = MeshComp->GetOwner()->FindComponentByClass<UAttackComponent>())
	{
		AttackComponent->OpenHitWindow({ HitSocketName }, EHitStopType::Light);
	}
}

void UAnimNotifyState_AttackHitWindow::NotifyEnd(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference
)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (UAttackComponent* AttackComponent = MeshComp->GetOwner()->FindComponentByClass<UAttackComponent>())
	{
		AttackComponent->CloseHitWindow();
	}
}
