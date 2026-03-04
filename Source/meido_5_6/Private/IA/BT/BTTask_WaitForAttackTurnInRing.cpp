#include "IA/BT/BTTask_WaitForAttackTurnInRing.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/MaidCharacter.h"
#include "IA/EnemyMaidAIController.h"

UBTTask_WaitForAttackTurnInRing::UBTTask_WaitForAttackTurnInRing()
{
	NodeName = TEXT("Wait For Attack Turn (Outer Ring)");
	bNotifyTick = true;
	bCreateNodeInstance = true;
	TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_WaitForAttackTurnInRing, TargetActorKey), AActor::StaticClass());
}

EBTNodeResult::Type UBTTask_WaitForAttackTurnInRing::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AEnemyMaidAIController* AIController = Cast<AEnemyMaidAIController>(OwnerComp.GetAIOwner());
	AMaidCharacter* Maid = Cast<AMaidCharacter>(AIController ? AIController->GetPawn() : nullptr);
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	AActor* TargetActor = Blackboard ? Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKey.SelectedKeyName)) : nullptr;

	if (!AIController || !Maid || !TargetActor)
	{
		return EBTNodeResult::Failed;
	}

	NextSlotRequestTime = 0.f;
	return EBTNodeResult::InProgress;
}

void UBTTask_WaitForAttackTurnInRing::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AEnemyMaidAIController* AIController = Cast<AEnemyMaidAIController>(OwnerComp.GetAIOwner());
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

	if (Distance > KINDA_SMALL_NUMBER)
	{
		const FRotator DesiredControlRotation = ToTarget.GetSafeNormal2D().Rotation();
		AIController->SetControlRotation(FRotator(0.f, DesiredControlRotation.Yaw, 0.f));
	}

	FVector DesiredMoveWorld = FVector::ZeroVector;
	const float SafeOuterMin = FMath::Max(0.f, FMath::Min(OuterRingMinRange, OuterRingMaxRange));
	const float SafeOuterMax = FMath::Max(SafeOuterMin, FMath::Max(OuterRingMinRange, OuterRingMaxRange));

	if (Distance > SafeOuterMax)
	{
		DesiredMoveWorld = ToTarget.GetSafeNormal2D();
	}
	else if (Distance < SafeOuterMin)
	{
		DesiredMoveWorld = -ToTarget.GetSafeNormal2D();
	}

	if (DesiredMoveWorld.SizeSquared2D() > KINDA_SMALL_NUMBER)
	{
		const FRotator ControlYaw(0.f, AIController->GetControlRotation().Yaw, 0.f);
		const FVector ForwardAxis = FRotationMatrix(ControlYaw).GetUnitAxis(EAxis::X);
		const FVector RightAxis = FRotationMatrix(ControlYaw).GetUnitAxis(EAxis::Y);
		const float ForwardInput = FMath::Clamp(FVector::DotProduct(DesiredMoveWorld, ForwardAxis), -1.f, 1.f);
		const float RightInput = FMath::Clamp(FVector::DotProduct(DesiredMoveWorld, RightAxis), -1.f, 1.f);
		Maid->DoMove(RightInput, ForwardInput);
	}
	else
	{
		Maid->DoMove(0.f, 0.f);
	}

	const float CurrentTime = Maid->GetWorld() ? Maid->GetWorld()->GetTimeSeconds() : 0.f;
	if (CurrentTime < NextSlotRequestTime)
	{
		return;
	}

	if (AIController->RequestAttackSlotForControlledPawn())
	{
		Maid->DoMove(0.f, 0.f);
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	NextSlotRequestTime = CurrentTime + FMath::Max(0.01f, SlotRetryInterval);
}

void UBTTask_WaitForAttackTurnInRing::OnTaskFinished(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory,
	EBTNodeResult::Type TaskResult
)
{
	AEnemyMaidAIController* AIController = Cast<AEnemyMaidAIController>(OwnerComp.GetAIOwner());
	AMaidCharacter* Maid = Cast<AMaidCharacter>(AIController ? AIController->GetPawn() : nullptr);
	if (Maid)
	{
		Maid->DoMove(0.f, 0.f);
	}

	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

