#include "IA/EnemyMaidAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BrainComponent.h"
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

	// AI is flow-driven by the player controller. Do not auto-start here.
	if (bAIEnabled)
	{
		SetAIEnabled(true);
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

	if (!bAIEnabled)
	{
		return;
	}

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

		APawn* ControlledPawn = GetPawn();
		if (bHasAttackSlot && ControlledPawn && AttackSlotKeepMaxDistance > 0.f)
		{
			const FVector ControlledLocation = ControlledPawn->GetActorLocation();
			const FVector TargetLocation = TargetPawn->GetActorLocation();
			const float DistanceToTarget = FVector::Dist2D(ControlledLocation, TargetLocation);
			if (DistanceToTarget > AttackSlotKeepMaxDistance)
			{
				ReleaseAttackSlot();
			}
		}
	}
	else
	{
		ClearFocus(EAIFocusPriority::Gameplay);
		if (bHasAttackSlot)
		{
			ReleaseAttackSlot();
		}
	}
}

bool AEnemyMaidAIController::RequestAttackSlotForControlledPawn()
{
	if (!bAIEnabled)
	{
		return false;
	}

	APawn* ControlledPawn = GetPawn();
	APawn* TargetPawn = CurrentTargetPawn.Get();
	if (!ControlledPawn || !TargetPawn)
	{
		return false;
	}

	if (AttackSlotRequestMaxDistance > 0.f)
	{
		const float DistanceToTarget = FVector::Dist2D(ControlledPawn->GetActorLocation(), TargetPawn->GetActorLocation());
		if (DistanceToTarget > AttackSlotRequestMaxDistance)
		{
			return false;
		}
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

void AEnemyMaidAIController::SetAIEnabled(const bool bEnabled)
{
	if (!bEnabled && !bAIEnabled)
	{
		return;
	}

	bAIEnabled = bEnabled;

	if (!bAIEnabled)
	{
		StopMovement();
		ClearFocus(EAIFocusPriority::Gameplay);
		ReleaseAttackSlot();

		if (UBrainComponent* Brain = BrainComponent)
		{
			Brain->StopLogic(TEXT("AI disabled by flow state"));
		}

		return;
	}

	NextTargetRefreshTime = 0.f;
	UpdateTarget();

	if (!GetPawn())
	{
		return;
	}

	if (BehaviorTreeAsset)
	{
		RunBehaviorTree(BehaviorTreeAsset);
	}

	if (Blackboard)
	{
		Blackboard->SetValueAsObject(TargetActorBlackboardKeyName, CurrentTargetPawn.Get());
	}
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
