// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/StateTypes.h"
#include "CharacterStateComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnCharacterOverallStateChanged,
	ECharacterOverallState, OldState,
	ECharacterOverallState, NewState
);

/**
 * Layered character posture (CP0.1 scaffold).
 * Systems write their slice; Overall is derived by priority.
 * Domain events (HP numbers, etc.) stay on owning systems — not here.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MEIDO_5_6_API UCharacterStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCharacterStateComponent();

	// --- Read ---
	UFUNCTION(BlueprintPure, Category = "State")
	ECharacterOverallState GetOverall() const { return OverallState; }

	UFUNCTION(BlueprintPure, Category = "State|Attack")
	EAttackState GetAttackState() const { return AttackState; }

	UFUNCTION(BlueprintPure, Category = "State|Health")
	EHealthActionState GetHealthState() const { return HealthState; }

	UFUNCTION(BlueprintPure, Category = "State|Locomotion")
	ELocomotionState GetLocomotionState() const { return LocomotionState; }

	UFUNCTION(BlueprintPure, Category = "State|MeiDou")
	EMeiDouLayerState GetMeiDouLayerState() const { return MeiDouLayerState; }

	UFUNCTION(BlueprintPure, Category = "State")
	bool IsDead() const { return HealthState == EHealthActionState::Dead; }

	UFUNCTION(BlueprintPure, Category = "State")
	bool IsStaggered() const { return HealthState == EHealthActionState::Stagger; }

	UFUNCTION(BlueprintPure, Category = "State")
	bool IsAttacking() const { return AttackState != EAttackState::None; }

	UFUNCTION(BlueprintPure, Category = "State")
	bool IsDashing() const { return LocomotionState == ELocomotionState::Dash; }

	UFUNCTION(BlueprintPure, Category = "State")
	bool IsMeiDouLocked() const
	{
		return MeiDouLayerState == EMeiDouLayerState::Active
			|| MeiDouLayerState == EMeiDouLayerState::Failed;
	}

	// --- Write (owning systems call these) ---
	UFUNCTION(BlueprintCallable, Category = "State|Attack")
	void SetAttackState(EAttackState NewState);

	UFUNCTION(BlueprintCallable, Category = "State|Health")
	void SetHealthState(EHealthActionState NewState);

	UFUNCTION(BlueprintCallable, Category = "State|Locomotion")
	void SetLocomotionState(ELocomotionState NewState);

	UFUNCTION(BlueprintCallable, Category = "State|MeiDou")
	void SetMeiDouLayerState(EMeiDouLayerState NewState);

	/** Reset slices to idle defaults (flow restart). */
	UFUNCTION(BlueprintCallable, Category = "State")
	void ResetToDefaults();

	UPROPERTY(BlueprintAssignable, Category = "State")
	FOnCharacterOverallStateChanged OnOverallStateChanged;

	/** On-screen debug for Overall / Attack / Health (CP0.1 smoke). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State|Debug")
	bool bDrawDebugState = false;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction
	) override;

private:
	void RecalculateOverall();
	void SetOverallInternal(ECharacterOverallState NewOverall);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", meta = (AllowPrivateAccess = "true"))
	ECharacterOverallState OverallState = ECharacterOverallState::Idle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State|Attack", meta = (AllowPrivateAccess = "true"))
	EAttackState AttackState = EAttackState::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State|Health", meta = (AllowPrivateAccess = "true"))
	EHealthActionState HealthState = EHealthActionState::Alive;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State|Locomotion", meta = (AllowPrivateAccess = "true"))
	ELocomotionState LocomotionState = ELocomotionState::Grounded;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State|MeiDou", meta = (AllowPrivateAccess = "true"))
	EMeiDouLayerState MeiDouLayerState = EMeiDouLayerState::Idle;
};
