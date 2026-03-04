#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_RequestAttackSlot.generated.h"

UCLASS()
class MEIDO_5_6_API UBTTask_RequestAttackSlot : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_RequestAttackSlot();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};

