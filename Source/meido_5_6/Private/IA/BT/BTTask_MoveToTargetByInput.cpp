#include "IA/BT/BTTask_MoveToTargetByInput.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/MaidCharacter.h"
#include "Engine/World.h"
#include "IA/EnemyMaidAIController.h"
#include "IA/MaidAIMovement.h"

UBTTask_MoveToTargetByInput::UBTTask_MoveToTargetByInput()
{
	NodeName = TEXT("Move To Target (Input)");
	bNotifyTick = true;
	bNotifyTaskFinished = true;
	bCreateNodeInstance = true;
	TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_MoveToTargetByInput, TargetActorKey), AActor::StaticClass());
}

EBTNodeResult::Type UBTTask_MoveToTargetByInput::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* TaskController = OwnerComp.GetAIOwner();
	AMaidCharacter* TaskMaid = Cast<AMaidCharacter>(TaskController ? TaskController->GetPawn() : nullptr);
	UBlackboardComponent* TaskBlackboard = OwnerComp.GetBlackboardComponent();
	AActor* TaskTarget = TaskBlackboard ? Cast<AActor>(TaskBlackboard->GetValueAsObject(TargetActorKey.SelectedKeyName)) : nullptr;
	if (!TaskController || !TaskMaid || !IsValid(TaskTarget) || TaskMaid->IsDead())
	{
		return EBTNodeResult::Failed;
	}
	CachedPathPoints.Reset();
	NextPathPointIndex = 1;
	NextRepathTime = 0.f;
	ApproachStartTime = TaskMaid->GetWorld()->GetTimeSeconds();
	return EBTNodeResult::InProgress;
}

void UBTTask_MoveToTargetByInput::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* TaskController = OwnerComp.GetAIOwner();
	AMaidCharacter* TaskMaid = Cast<AMaidCharacter>(TaskController ? TaskController->GetPawn() : nullptr);
	UBlackboardComponent* TaskBlackboard = OwnerComp.GetBlackboardComponent();
	AActor* TaskTarget = TaskBlackboard ? Cast<AActor>(TaskBlackboard->GetValueAsObject(TargetActorKey.SelectedKeyName)) : nullptr;
	if (!TaskController || !TaskMaid || !IsValid(TaskTarget) || TaskMaid->IsDead())
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}
	if (const AEnemyMaidAIController* EnemyController = Cast<AEnemyMaidAIController>(TaskController))
	{
		if (!EnemyController->IsAIEnabled() || !EnemyController->HasAttackSlotForControlledPawn())
		{
			FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
			return;
		}
	}
	if (TaskMaid->GetWorld()->GetTimeSeconds() - ApproachStartTime >= FMath::Max(0.1f, MaxApproachSeconds))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}
	if (FVector::Dist2D(TaskMaid->GetActorLocation(), TaskTarget->GetActorLocation()) <= FMath::Max(0.f, AcceptableRange))
	{
		MeidoAIMovement::ApplyInput(TaskMaid, TaskController, FVector::ZeroVector, TaskTarget->GetActorLocation());
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}
	const FVector ApproachDirection = MeidoAIMovement::ResolveDirection(TaskMaid, TaskTarget->GetActorLocation(),
		RepathInterval, PathPointAcceptanceRadius, NextRepathTime, NextPathPointIndex, CachedPathPoints);
	MeidoAIMovement::ApplyInput(TaskMaid, TaskController, ApproachDirection, TaskTarget->GetActorLocation());
}

void UBTTask_MoveToTargetByInput::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	AAIController* TaskController = OwnerComp.GetAIOwner();
	if (AMaidCharacter* TaskMaid = Cast<AMaidCharacter>(TaskController ? TaskController->GetPawn() : nullptr))
	{
		TaskMaid->DoMove(0.f, 0.f);
	}
	if (TaskResult != EBTNodeResult::Succeeded)
	{
		if (AEnemyMaidAIController* EnemyController = Cast<AEnemyMaidAIController>(TaskController))
		{
			EnemyController->ReleaseAttackSlotForControlledPawn();
		}
	}
	CachedPathPoints.Reset();
	NextPathPointIndex = 1;
	NextRepathTime = 0.f;
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}
