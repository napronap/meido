#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BTTask_MoveToTargetByInput.generated.h"

UCLASS()
class MEIDO_5_6_API UBTTask_MoveToTargetByInput : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_MoveToTargetByInput();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;

	UPROPERTY(EditAnywhere, Category="Task")
	FBlackboardKeySelector TargetActorKey;

	UPROPERTY(EditAnywhere, Category="Task", meta=(ClampMin="0.0", UIMin="0.0"))
	float AcceptableRange = 220.f;
};

