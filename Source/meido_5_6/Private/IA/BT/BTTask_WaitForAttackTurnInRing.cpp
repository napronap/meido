#include "IA/BT/BTTask_WaitForAttackTurnInRing.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/MaidCharacter.h"
#include "Engine/World.h"
#include "IA/EnemyMaidAIController.h"
#include "IA/MaidAIMovement.h"

UBTTask_WaitForAttackTurnInRing::UBTTask_WaitForAttackTurnInRing()
{
	NodeName = TEXT("Hold Around Target / Wait For Attack Turn");
	bNotifyTick = true;
	bNotifyTaskFinished = true;
	bCreateNodeInstance = true;
	TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_WaitForAttackTurnInRing, TargetActorKey), AActor::StaticClass());
}

EBTNodeResult::Type UBTTask_WaitForAttackTurnInRing::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AEnemyMaidAIController* TaskController = Cast<AEnemyMaidAIController>(OwnerComp.GetAIOwner());
	AMaidCharacter* TaskMaid = Cast<AMaidCharacter>(TaskController ? TaskController->GetPawn() : nullptr);
	UBlackboardComponent* TaskBlackboard = OwnerComp.GetBlackboardComponent();
	AActor* TaskTarget = TaskBlackboard ? Cast<AActor>(TaskBlackboard->GetValueAsObject(TargetActorKey.SelectedKeyName)) : nullptr;
	if (!TaskController || !TaskMaid || !IsValid(TaskTarget) || TaskMaid->IsDead())
	{
		return EBTNodeResult::Failed;
	}
	const float HoldTime = TaskMaid->GetWorld()->GetTimeSeconds();
	NextSlotRequestTime = HoldTime + FMath::FRandRange(0.2f, 0.8f);
	NextOrbitChangeTime = HoldTime + FMath::FRandRange(0.2f, 1.f);
	bOrbitResting = FMath::RandBool();
	bCorrectingHoldRadius = false;
	NextRepathTime = 0.f;
	NextPathPointIndex = 1;
	CachedPathPoints.Reset();
	return EBTNodeResult::InProgress;
}

void UBTTask_WaitForAttackTurnInRing::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AEnemyMaidAIController* TaskController = Cast<AEnemyMaidAIController>(OwnerComp.GetAIOwner());
	AMaidCharacter* TaskMaid = Cast<AMaidCharacter>(TaskController ? TaskController->GetPawn() : nullptr);
	UBlackboardComponent* TaskBlackboard = OwnerComp.GetBlackboardComponent();
	AActor* TaskTarget = TaskBlackboard ? Cast<AActor>(TaskBlackboard->GetValueAsObject(TargetActorKey.SelectedKeyName)) : nullptr;
	if (!TaskController || !TaskMaid || !IsValid(TaskTarget) || TaskMaid->IsDead() || !TaskController->IsAIEnabled())
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}
	const float HoldTime = TaskMaid->GetWorld()->GetTimeSeconds();
	if (TaskController->HasAttackSlotForControlledPawn()
		|| (HoldTime >= NextSlotRequestTime && TaskController->RequestAttackSlotForControlledPawn()))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}
	if (HoldTime >= NextSlotRequestTime)
	{
		NextSlotRequestTime = HoldTime + FMath::Max(0.01f, SlotRetryInterval) * FMath::FRandRange(0.8f, 1.4f);
	}
	if (HoldTime >= NextOrbitChangeTime)
	{
		bOrbitResting = !bOrbitResting;
		const float SafeIdleMin = FMath::Max(0.05f, FMath::Min(HoldIdleMinSeconds, HoldIdleMaxSeconds));
		const float SafeIdleMax = FMath::Max(SafeIdleMin, FMath::Max(HoldIdleMinSeconds, HoldIdleMaxSeconds));
		NextOrbitChangeTime = HoldTime + FMath::FRandRange(SafeIdleMin, SafeIdleMax);
	}

	const FVector TargetPosition = TaskTarget->GetActorLocation();
	const FVector MaidPosition = TaskMaid->GetActorLocation();
	FVector OutwardDirection = (MaidPosition - TargetPosition).GetSafeNormal2D();
	if (OutwardDirection.IsNearlyZero())
	{
		OutwardDirection = TaskMaid->GetActorRightVector().GetSafeNormal2D();
	}
	const float LegacyMin = FMath::Max(0.f, FMath::Min(OuterRingMinRange, OuterRingMaxRange));
	const float LegacyMax = FMath::Max(LegacyMin, FMath::Max(OuterRingMinRange, OuterRingMaxRange));
	const float DesiredRadius = bUsePersonalHoldRadius ? TaskController->GetPreferredHoldRadius() : (LegacyMin + LegacyMax) * 0.5f;
	const float BandWidth = bUsePersonalHoldRadius ? FMath::Max(10.f, HoldBandHalfWidth) : FMath::Max(10.f, (LegacyMax - LegacyMin) * 0.5f);
	const float RadiusError = FVector::Dist2D(MaidPosition, TargetPosition) - DesiredRadius;
	if (FMath::Abs(RadiusError) > BandWidth + FMath::Max(0.f, HoldHysteresis))
	{
		bCorrectingHoldRadius = true;
	}
	else if (FMath::Abs(RadiusError) < BandWidth * 0.5f)
	{
		bCorrectingHoldRadius = false;
	}

	FVector HoldDestination = MaidPosition;
	float HoldInputScale = 0.f;
	if (bCorrectingHoldRadius)
	{
		HoldDestination = TargetPosition + OutwardDirection * DesiredRadius;
		HoldInputScale = FMath::Clamp(FMath::Abs(RadiusError) / FMath::Max(1.f, DesiredRadius), 0.3f, 1.f);
	}
	else if (!bOrbitResting && TaskController->UsesPreferredHoldAngle())
	{
		const float HoldRadians = FMath::DegreesToRadians(TaskController->GetPreferredHoldAngle());
		HoldDestination = TargetPosition + FVector(FMath::Cos(HoldRadians), FMath::Sin(HoldRadians), 0.f) * DesiredRadius;
		HoldInputScale = FVector::Dist2D(MaidPosition, HoldDestination) > 80.f ? 0.35f : 0.f;
	}
	else if (!bOrbitResting)
	{
		const FVector OrbitTangent(-OutwardDirection.Y, OutwardDirection.X, 0.f);
		HoldDestination = MaidPosition + OrbitTangent * TaskController->GetHoldOrbitDirection() * 140.f;
		HoldInputScale = FMath::Clamp(OrbitInputScale, 0.f, 1.f);
	}

	FVector HoldInput = FVector::ZeroVector;
	if (HoldInputScale > 0.f)
	{
		HoldInput = MeidoAIMovement::ResolveDirection(TaskMaid, HoldDestination, RepathInterval,
			PathPointAcceptanceRadius, NextRepathTime, NextPathPointIndex, CachedPathPoints) * HoldInputScale;
	}
	else
	{
		CachedPathPoints.Reset();
		NextRepathTime = 0.f;
	}
	MeidoAIMovement::ApplyInput(TaskMaid, TaskController, HoldInput, TargetPosition);
}

void UBTTask_WaitForAttackTurnInRing::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	AEnemyMaidAIController* TaskController = Cast<AEnemyMaidAIController>(OwnerComp.GetAIOwner());
	if (AMaidCharacter* TaskMaid = Cast<AMaidCharacter>(TaskController ? TaskController->GetPawn() : nullptr))
	{
		TaskMaid->DoMove(0.f, 0.f);
	}
	if (TaskController && TaskResult != EBTNodeResult::Succeeded)
	{
		TaskController->ReleaseAttackSlotForControlledPawn();
	}
	CachedPathPoints.Reset();
	NextPathPointIndex = 1;
	NextRepathTime = 0.f;
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}
