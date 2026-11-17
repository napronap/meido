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

	UFUNCTION(BlueprintPure, Category = "AI|Hold")
	float GetPreferredHoldRadius() const { return PreferredHoldRadius; }

	float GetHoldOrbitDirection() const { return HoldOrbitDirection; }
	float GetPreferredHoldAngle() const { return PreferredHoldAngle; }
	bool UsesPreferredHoldAngle() const { return bUsePreferredHoldAngle; }

	/** Phase 2 hooks only; no automatic dash/combo continuation in the base plan. */
	UFUNCTION(BlueprintPure, Category = "AI|Future")
	bool CanUseDashApproach() const { return bCanDashApproach; }

	UFUNCTION(BlueprintPure, Category = "AI|Future")
	float GetComboFollowUpChance() const { return ComboFollowUpChance; }

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditDefaultsOnly, Category = "AI|Hold", meta = (ClampMin = "0.0"))
	float PreferredHoldRadiusMin = 300.f;

	UPROPERTY(EditDefaultsOnly, Category = "AI|Hold", meta = (ClampMin = "0.0"))
	float PreferredHoldRadiusMax = 480.f;

	UPROPERTY(EditDefaultsOnly, Category = "AI|Hold")
	bool bUsePreferredHoldAngle = false;

	UPROPERTY(EditDefaultsOnly, Category = "AI|Future")
	bool bCanDashApproach = false;

	UPROPERTY(EditDefaultsOnly, Category = "AI|Future", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ComboFollowUpChance = 0.f;

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
	float PreferredHoldRadius = 390.f;
	float HoldOrbitDirection = 1.f;
	float PreferredHoldAngle = 0.f;
	TWeakObjectPtr<APawn> CurrentTargetPawn;
	float NextTargetRefreshTime = 0.f;
	bool bHasAttackSlot = false;
	// Default on so playground spawn fights without Demo flow. Demo still disables in menu/pause/win/lose.
	// TODO: reassess with proper AI/flow ownership
	bool bAIEnabled = true;

	void UpdateTarget();
	void ReleaseAttackSlot();
};
