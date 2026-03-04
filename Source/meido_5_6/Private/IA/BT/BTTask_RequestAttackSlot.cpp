#include "IA/BT/BTTask_RequestAttackSlot.h"
#include "IA/EnemyMaidAIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

UBTTask_RequestAttackSlot::UBTTask_RequestAttackSlot()
{
	NodeName = TEXT("Request Attack Slot");
}

EBTNodeResult::Type UBTTask_RequestAttackSlot::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AEnemyMaidAIController* AIController = Cast<AEnemyMaidAIController>(OwnerComp.GetAIOwner());
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	return AIController->RequestAttackSlotForControlledPawn()
		? EBTNodeResult::Succeeded
		: EBTNodeResult::Failed;
}

