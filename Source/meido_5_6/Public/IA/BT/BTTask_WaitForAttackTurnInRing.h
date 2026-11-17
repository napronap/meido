#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BTTask_WaitForAttackTurnInRing.generated.h"

UCLASS()
class MEIDO_5_6_API UBTTask_WaitForAttackTurnInRing : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_WaitForAttackTurnInRing();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;

	UPROPERTY(EditAnywhere, Category="Task")
	FBlackboardKeySelector TargetActorKey;

	UPROPERTY(EditAnywhere, Category="Task", meta=(ClampMin="0.0", UIMin="0.0"))
	float OuterRingMinRange = 320.f;

	UPROPERTY(EditAnywhere, Category="Task", meta=(ClampMin="0.0", UIMin="0.0"))
	float OuterRingMaxRange = 460.f;

	UPROPERTY(EditAnywhere, Category="Task", meta=(ClampMin="0.01", UIMin="0.01"))
	float SlotRetryInterval = 0.25f;

	/** False keeps the old OuterRingMin/Max tuning available to existing BT assets. */
	UPROPERTY(EditAnywhere, Category = "Task|Hold")
	bool bUsePersonalHoldRadius = true;

	UPROPERTY(EditAnywhere, Category = "Task|Hold", meta = (ClampMin = "10.0"))
	float HoldBandHalfWidth = 65.f;

	UPROPERTY(EditAnywhere, Category = "Task|Hold", meta = (ClampMin = "0.0"))
	float HoldHysteresis = 25.f;

	UPROPERTY(EditAnywhere, Category = "Task|Hold", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float OrbitInputScale = 0.2f;

	UPROPERTY(EditAnywhere, Category = "Task|Hold", meta = (ClampMin = "0.05"))
	float HoldIdleMinSeconds = 0.2f;

	UPROPERTY(EditAnywhere, Category = "Task|Hold", meta = (ClampMin = "0.05"))
	float HoldIdleMaxSeconds = 1.f;

	UPROPERTY(EditAnywhere, Category="Task|Pathfinding", meta=(ClampMin="0.05", UIMin="0.05"))
	float RepathInterval = 0.25f;

	UPROPERTY(EditAnywhere, Category="Task|Pathfinding", meta=(ClampMin="5.0", UIMin="5.0"))
	float PathPointAcceptanceRadius = 50.f;

private:
	bool bCorrectingHoldRadius = false;
	bool bOrbitResting = false;
	float NextOrbitChangeTime = 0.f;
	float NextSlotRequestTime = 0.f;
	float NextRepathTime = 0.f;
	int32 NextPathPointIndex = 1;
	TArray<FVector> CachedPathPoints;
};
