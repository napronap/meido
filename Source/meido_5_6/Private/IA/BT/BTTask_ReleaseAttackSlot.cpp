#include "IA/BT/BTTask_ReleaseAttackSlot.h"
#include "Characters/MaidCharacter.h"
#include "IA/EnemyMaidAIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

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
		AMaidCharacter* Maid = Cast<AMaidCharacter>(AIController->GetPawn());
		if (Maid)
		{
			const ECharacterState State = Maid->GetCharacterState();
			if (State == ECharacterState::ECS_Attacking || State == ECharacterState::ECS_Recovering)
			{
				return EBTNodeResult::InProgress;
			}
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
		AMaidCharacter* Maid = Cast<AMaidCharacter>(AIController->GetPawn());
		if (Maid)
		{
			const ECharacterState State = Maid->GetCharacterState();
			if (State == ECharacterState::ECS_Attacking || State == ECharacterState::ECS_Recovering)
			{
				return;
			}
		}
	}

	AIController->ReleaseAttackSlotForControlledPawn();
	FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
}
