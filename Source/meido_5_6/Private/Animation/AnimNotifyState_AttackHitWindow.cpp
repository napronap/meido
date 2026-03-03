// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotifyState_AttackHitWindow.h"
#include "ActorComponents/AttackComponent.h"
#include "GameFramework/Character.h"

void UAnimNotifyState_AttackHitWindow::NotifyBegin(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float TotalDuration,
	const FAnimNotifyEventReference& EventReference
)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (!MeshComp) return;

	ACharacter* Character = Cast<ACharacter>(MeshComp->GetOwner());
	if (!Character) return;

	UAttackComponent* AttackComponent = Character->FindComponentByClass<UAttackComponent>();

	if (AttackComponent)
	{
		if (HitSocketNames.Num() > 0)
		{
			AttackComponent->OpenHitWindowSockets(HitSocketNames);
		}
		else
		{
			AttackComponent->OpenHitWindow(HitSocketName);
		}
	}
}

void UAnimNotifyState_AttackHitWindow::NotifyEnd(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference
)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (!MeshComp) return;

	ACharacter* Character = Cast<ACharacter>(MeshComp->GetOwner());
	if (!Character) return;

	UAttackComponent* AttackComp =
		Character->FindComponentByClass<UAttackComponent>();

	if (AttackComp)
	{
		AttackComp->CloseHitWindow();
	}
}
