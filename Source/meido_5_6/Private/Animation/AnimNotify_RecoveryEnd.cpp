// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotify_RecoveryEnd.h"


#include "Interfaces/ComboAttacker.h"

void UAnimNotify_RecoveryEnd::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference
)
{
	Super::Notify(MeshComp, Animation, EventReference);

	AActor* Owner = MeshComp->GetOwner();

	if (!Owner) return;

	if (Owner->Implements<UComboAttacker>())
	{
		IComboAttacker::Execute_RecoveryEnd(Owner);
	}
}
