#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_ReleaseAttackSlot.generated.h"

UCLASS()
class MEIDO_5_6_API UBTTask_ReleaseAttackSlot : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_ReleaseAttackSlot();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, Category="Task")
	bool bWaitForIdleBeforeRelease = true;
};
