#include "IA/BT/BTTask_MoveToTargetByInput.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/MaidCharacter.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"

UBTTask_MoveToTargetByInput::UBTTask_MoveToTargetByInput()
{
	NodeName = TEXT("Move To Target (Input)");
	bNotifyTick = true;
	bCreateNodeInstance = true;
	TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_MoveToTargetByInput, TargetActorKey), AActor::StaticClass());
}

EBTNodeResult::Type UBTTask_MoveToTargetByInput::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	AMaidCharacter* Maid = Cast<AMaidCharacter>(AIController ? AIController->GetPawn() : nullptr);
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	AActor* TargetActor = Blackboard ? Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKey.SelectedKeyName)) : nullptr;

	if (!AIController || !Maid || !TargetActor)
	{
		return EBTNodeResult::Failed;
	}

	CachedPathPoints.Reset();
	NextPathPointIndex = 1;
	NextRepathTime = 0.f;

	return EBTNodeResult::InProgress;
}

void UBTTask_MoveToTargetByInput::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	AMaidCharacter* Maid = Cast<AMaidCharacter>(AIController ? AIController->GetPawn() : nullptr);
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	AActor* TargetActor = Blackboard ? Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKey.SelectedKeyName)) : nullptr;

	if (!AIController || !Maid || !TargetActor || Maid->IsDead())
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	FVector ToTarget = TargetActor->GetActorLocation() - Maid->GetActorLocation();
	ToTarget.Z = 0.f;

	const float Distance = ToTarget.Size();
	if (Distance <= AcceptableRange)
	{
		Maid->DoMove(0.f, 0.f);
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	UWorld* World = Maid->GetWorld();
	const float CurrentTime = World ? World->GetTimeSeconds() : 0.f;
	const bool bNeedsRepath = (CurrentTime >= NextRepathTime) || (CachedPathPoints.Num() < 2) || (NextPathPointIndex >= CachedPathPoints.Num());

	if (bNeedsRepath)
	{
		CachedPathPoints.Reset();
		NextPathPointIndex = 1;

		if (World)
		{
			if (UNavigationPath* NavPath = UNavigationSystemV1::FindPathToActorSynchronously(
				World,
				Maid->GetActorLocation(),
				TargetActor,
				AcceptableRange,
				Maid))
			{
				CachedPathPoints = NavPath->PathPoints;
			}
		}

		NextRepathTime = CurrentTime + FMath::Max(0.05f, RepathInterval);
	}

	FVector MoveDir = ToTarget.GetSafeNormal2D();
	while (CachedPathPoints.IsValidIndex(NextPathPointIndex))
	{
		FVector ToPathPoint = CachedPathPoints[NextPathPointIndex] - Maid->GetActorLocation();
		ToPathPoint.Z = 0.f;
		if (ToPathPoint.SizeSquared2D() <= FMath::Square(PathPointAcceptanceRadius))
		{
			++NextPathPointIndex;
			continue;
		}

		MoveDir = ToPathPoint.GetSafeNormal2D();
		break;
	}

	if (MoveDir.IsNearlyZero())
	{
		MoveDir = ToTarget.GetSafeNormal2D();
	}

	if (MoveDir.IsNearlyZero())
	{
		Maid->DoMove(0.f, 0.f);
		return;
	}

	const FRotator DesiredControlRotation = MoveDir.Rotation();
	AIController->SetControlRotation(FRotator(0.f, DesiredControlRotation.Yaw, 0.f));

	const FRotator ControlYaw(0.f, AIController->GetControlRotation().Yaw, 0.f);
	const FVector ForwardAxis = FRotationMatrix(ControlYaw).GetUnitAxis(EAxis::X);
	const FVector RightAxis = FRotationMatrix(ControlYaw).GetUnitAxis(EAxis::Y);

	const float ForwardInput = FMath::Clamp(FVector::DotProduct(MoveDir, ForwardAxis), -1.f, 1.f);
	const float RightInput = FMath::Clamp(FVector::DotProduct(MoveDir, RightAxis), -1.f, 1.f);
	Maid->DoMove(RightInput, ForwardInput);
}

void UBTTask_MoveToTargetByInput::OnTaskFinished(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory,
	EBTNodeResult::Type TaskResult
)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	AMaidCharacter* Maid = Cast<AMaidCharacter>(AIController ? AIController->GetPawn() : nullptr);
	if (Maid)
	{
		Maid->DoMove(0.f, 0.f);
	}
	CachedPathPoints.Reset();
	NextPathPointIndex = 1;
	NextRepathTime = 0.f;

	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}
