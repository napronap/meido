#include "IA/BT/BTTask_ReleaseAttackSlot.h"
#include "ActorComponents/CharacterStateComponent.h"
#include "Characters/MaidCharacter.h"
#include "IA/EnemyMaidAIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

namespace
{
	/** Still in attack chain or hit-stun — hold the shared attack slot. */
	bool ShouldHoldAttackSlot(const AMaidCharacter* Maid)
	{
		if (!Maid)
		{
			return false;
		}

		const UCharacterStateComponent* State = Maid->GetCharacterStateComponent();
		if (!State)
		{
			return false;
		}

		return State->IsAttacking() || State->IsStaggered();
	}
}

UBTTask_ReleaseAttackSlot::UBTTask_ReleaseAttackSlot()
{
	NodeName = TEXT("Release Attack Slot");
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_ReleaseAttackSlot::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AEnemyMaidAIController* AIController = Cast<AEnemyMaidAIController>(OwnerComp.GetAIOwner());
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	if (bWaitForIdleBeforeRelease)
	{
		const AMaidCharacter* Maid = Cast<AMaidCharacter>(AIController->GetPawn());
		if (ShouldHoldAttackSlot(Maid))
		{
			return EBTNodeResult::InProgress;
		}
	}

	AIController->ReleaseAttackSlotForControlledPawn();
	return EBTNodeResult::Succeeded;
}

void UBTTask_ReleaseAttackSlot::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AEnemyMaidAIController* AIController = Cast<AEnemyMaidAIController>(OwnerComp.GetAIOwner());
	if (!AIController)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	if (bWaitForIdleBeforeRelease)
	{
		const AMaidCharacter* Maid = Cast<AMaidCharacter>(AIController->GetPawn());
		if (ShouldHoldAttackSlot(Maid))
		{
			return;
		}
	}

	AIController->ReleaseAttackSlotForControlledPawn();
	FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
}
