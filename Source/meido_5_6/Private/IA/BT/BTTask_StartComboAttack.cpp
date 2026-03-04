#include "IA/BT/BTTask_StartComboAttack.h"
#include "Characters/MaidCharacter.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "IA/EnemyMaidAIController.h"

UBTTask_StartComboAttack::UBTTask_StartComboAttack()
{
	NodeName = TEXT("Start Combo Attack");
}

EBTNodeResult::Type UBTTask_StartComboAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AEnemyMaidAIController* AIController = Cast<AEnemyMaidAIController>(OwnerComp.GetAIOwner());
	AMaidCharacter* Maid = Cast<AMaidCharacter>(AIController ? AIController->GetPawn() : nullptr);
	if (!AIController || !Maid || Maid->IsDead())
	{
		return EBTNodeResult::Failed;
	}

	if (bRequireAttackSlot && !AIController->HasAttackSlotForControlledPawn())
	{
		return EBTNodeResult::Failed;
	}

	const ECharacterState PreviousState = Maid->GetCharacterState();
	Maid->DoStartComboAttack();

	return (Maid->GetCharacterState() == ECharacterState::ECS_Attacking || PreviousState == ECharacterState::ECS_Attacking)
		? EBTNodeResult::Succeeded
		: EBTNodeResult::Failed;
}
