#include "IA/EnemyMaidAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/MaidCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "IA/CombatDirectorSubsystem.h"
#include "Kismet/GameplayStatics.h"

AEnemyMaidAIController::AEnemyMaidAIController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AEnemyMaidAIController::BeginPlay()
{
	Super::BeginPlay();
	UpdateTarget();
}

void AEnemyMaidAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (AMaidCharacter* ControlledMaid = Cast<AMaidCharacter>(InPawn))
	{
		ControlledMaid->bUseControllerRotationYaw = true;
		if (UCharacterMovementComponent* MovementComponent = ControlledMaid->GetCharacterMovement())
		{
			MovementComponent->bOrientRotationToMovement = false;
		}
	}

	bHasAttackSlot = false;
	NextTargetRefreshTime = 0.f;
	UpdateTarget();

	if (BehaviorTreeAsset)
	{
		RunBehaviorTree(BehaviorTreeAsset);
	}

	if (Blackboard)
	{
		Blackboard->SetValueAsObject(TargetActorBlackboardKeyName, CurrentTargetPawn.Get());
	}
}

void AEnemyMaidAIController::OnUnPossess()
{
	ClearFocus(EAIFocusPriority::Gameplay);
	ReleaseAttackSlot();
	Super::OnUnPossess();
}

void AEnemyMaidAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!GetPawn())
	{
		ReleaseAttackSlot();
		return;
	}

	const float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	if (CurrentTime >= NextTargetRefreshTime)
	{
		NextTargetRefreshTime = CurrentTime + TargetRefreshInterval;
		UpdateTarget();
		if (Blackboard)
		{
			Blackboard->SetValueAsObject(TargetActorBlackboardKeyName, CurrentTargetPawn.Get());
		}
	}

	if (APawn* TargetPawn = CurrentTargetPawn.Get())
	{
		SetFocus(TargetPawn, EAIFocusPriority::Gameplay);
	}
	else
	{
		ClearFocus(EAIFocusPriority::Gameplay);
	}
}

bool AEnemyMaidAIController::RequestAttackSlotForControlledPawn()
{
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn)
	{
		return false;
	}

	UCombatDirectorSubsystem* CombatDirector = GetGameInstance()
		? GetGameInstance()->GetSubsystem<UCombatDirectorSubsystem>()
		: nullptr;
	if (!CombatDirector)
	{
		return false;
	}

	const bool bGranted = CombatDirector->RequestAttackSlot(ControlledPawn);
	bHasAttackSlot = bGranted;
	return bGranted;
}

void AEnemyMaidAIController::ReleaseAttackSlotForControlledPawn()
{
	ReleaseAttackSlot();
}

bool AEnemyMaidAIController::HasAttackSlotForControlledPawn() const
{
	return bHasAttackSlot;
}

void AEnemyMaidAIController::UpdateTarget()
{
	CurrentTargetPawn = UGameplayStatics::GetPlayerPawn(this, 0);
}

void AEnemyMaidAIController::ReleaseAttackSlot()
{
	if (!bHasAttackSlot)
	{
		return;
	}

	APawn* ControlledPawn = GetPawn();
	UCombatDirectorSubsystem* CombatDirector = GetGameInstance()
		? GetGameInstance()->GetSubsystem<UCombatDirectorSubsystem>()
		: nullptr;

	if (CombatDirector && ControlledPawn)
	{
		CombatDirector->ReleaseAttackSlot(ControlledPawn);
	}

	bHasAttackSlot = false;
}

