#include "IA/BT/BTTask_StartComboAttack.h"
#include "ActorComponents/CharacterStateComponent.h"
#include "Characters/MaidCharacter.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "IA/EnemyMaidAIController.h"
#include "Types/StateTypes.h"

namespace
{
	bool IsInActiveComboAttack(const UCharacterStateComponent* State)
	{
		if (!State || !State->IsAttacking())
		{
			return false;
		}

		// Whiff recover is still "attack slice" but AI should not treat it as a live combo.
		return State->GetAttackState() != EAttackState::WhiffRecover;
	}
}

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

	const UCharacterStateComponent* State = Maid->GetCharacterStateComponent();
	const bool bWasAttacking = IsInActiveComboAttack(State);
	Maid->DoStartComboAttack();
	const bool bNowAttacking = IsInActiveComboAttack(Maid->GetCharacterStateComponent());

	return (bNowAttacking || bWasAttacking)
		? EBTNodeResult::Succeeded
		: EBTNodeResult::Failed;
}
