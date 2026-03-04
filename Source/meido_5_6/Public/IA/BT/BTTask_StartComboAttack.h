#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_StartComboAttack.generated.h"

UCLASS()
class MEIDO_5_6_API UBTTask_StartComboAttack : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_StartComboAttack();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category="Task")
	bool bRequireAttackSlot = true;
};
