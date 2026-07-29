#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyMaidAIController.generated.h"

class AMaidCharacter;
class UBehaviorTree;

UCLASS()
class MEIDO_5_6_API AEnemyMaidAIController : public AAIController
{
	GENERATED_BODY()

public:
	AEnemyMaidAIController();

	UFUNCTION(BlueprintCallable, Category="AI")
	void SetAIEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure, Category="AI")
	bool IsAIEnabled() const { return bAIEnabled; }

	// Request/release combat slot helpers for BT tasks or BP logic
	UFUNCTION(BlueprintCallable, Category="AI|Combat")
	bool RequestAttackSlotForControlledPawn();

	UFUNCTION(BlueprintCallable, Category="AI|Combat")
	void ReleaseAttackSlotForControlledPawn();

	UFUNCTION(BlueprintPure, Category="AI|Combat")
	bool HasAttackSlotForControlledPawn() const;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	// Main behavior entrypoint. AI flow is done in BT blueprint
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="AI|BehaviorTree")
	UBehaviorTree* BehaviorTreeAsset = nullptr;

	// Blackboard key updated with current target pawn while BT mode is active
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="AI|BehaviorTree")
	FName TargetActorBlackboardKeyName = TEXT("TargetActor");

	UPROPERTY(EditAnywhere, Category="AI|BehaviorTree", meta=(ClampMin="0.05", UIMin="0.05"))
	float TargetRefreshInterval = 0.25f;

	// Prevent far enemies from reserving an attack slot (will probably make private)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI|Combat", meta=(ClampMin="0.0", UIMin="0.0"))
	float AttackSlotRequestMaxDistance = 520.f;

	// If the enemy drifts too far while holding a slot, it releases it
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI|Combat", meta=(ClampMin="0.0", UIMin="0.0"))
	float AttackSlotKeepMaxDistance = 700.f;

private:
	TWeakObjectPtr<APawn> CurrentTargetPawn;
	float NextTargetRefreshTime = 0.f;
	bool bHasAttackSlot = false;
	// Default on so playground spawn fights without Demo flow. Demo still disables in menu/pause/win/lose.
	// TODO: reassess with proper AI/flow ownership
	bool bAIEnabled = true;

	void UpdateTarget();
	void ReleaseAttackSlot();
};
