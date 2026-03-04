#include "IA/BT/BTTask_MoveToTargetByInput.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/MaidCharacter.h"
#include "AIController.h"

UBTTask_MoveToTargetByInput::UBTTask_MoveToTargetByInput()
{
	NodeName = TEXT("Move To Target (Input)");
	bNotifyTick = true;
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

	const FRotator DesiredControlRotation = ToTarget.GetSafeNormal2D().Rotation();
	AIController->SetControlRotation(FRotator(0.f, DesiredControlRotation.Yaw, 0.f));

	const FVector MoveDir = ToTarget.GetSafeNormal2D();
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

	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

