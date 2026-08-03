// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Components/ActorComponent.h"
#include "DashComponent.generated.h"

class AMaidCharacter;
class UCharacterMovementComponent;

/**
 * Dash: C++ owns request, gates, direction, cooldown, anim flags.
 * Displacement comes from animation root motion (CP2.1) — no velocity override tick.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MEIDO_5_6_API UDashComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDashComponent();

	UFUNCTION(BlueprintCallable, Category = "Dash")
	bool TryDash(const FVector2D& MoveInput, const FRotator& ControlRotation, bool bLockOnActive);

	UFUNCTION(BlueprintCallable, Category = "Dash")
	void CancelDash();

	UFUNCTION(BlueprintPure, Category = "Dash")
	bool IsDashing() const { return bIsDashing; }

	/** Called from anim notify when dash clip ends early (optional). */
	UFUNCTION(BlueprintCallable, Category = "Dash")
	void NotifyDashAnimationEnded();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction
	) override;

	/**
	 * Max time bIsDashing stays true (safety / end of state).
	 * Distance comes from root motion, not Speed*Duration.
	 */
	UPROPERTY(EditAnywhere, Category = "Dash", meta = (ClampMin = "0.01", UIMin = "0.01"))
	float DashDuration = 0.22f;

	UPROPERTY(EditAnywhere, Category = "Dash", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float DashCooldown = 0.35f;

	UPROPERTY(EditAnywhere, Category = "Dash", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinDirectionInput = 0.15f;

	/** Scale applied to anim root motion while dashing (1 = as authored). */
	UPROPERTY(EditAnywhere, Category = "Dash|RootMotion", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float DashRootMotionScale = 1.f;

	/**
	 * Experimental: while dashing, set AnimInstance RootMotionMode to Everything
	 * so a state-machine / blend-space dash can apply RM under "Montages Only" ABP defaults.
	 * Restores previous mode on End. If loco has RM, may hitch for one frame — test in PIE.
	 */
	UPROPERTY(EditAnywhere, Category = "Dash|RootMotion")
	bool bForceRootMotionFromEverythingWhileDashing = true;

	/**
	 * Legacy: if true, GrantIFrames(DashIFrameDuration) on start (player).
	 * Prefer AnimNotifyState_DashIFrames on the dash anim; leave false when notifies are authored.
	 */
	UPROPERTY(EditAnywhere, Category = "Dash|IFrames")
	bool bUseLegacyTimedIFramesOnStart = true;

private:
	bool CanDash() const;
	FVector ComputeDashWorldDirection(const FVector2D& MoveInput, const FRotator& ControlRotation) const;
	FVector2D ComputeDashAnimDirection(const FVector& WorldDirection, const FRotator& ControlRotation) const;
	void StartDash(const FVector& InDashDirection, const FVector2D& AnimDirection);
	void EndDash();
	void UpdateDashAnimState(bool bDashing, const FVector2D& AnimDirection) const;
	UAnimInstance* GetOwnerAnimInstance() const;
	void ApplyDashRootMotionMode();
	void RestoreRootMotionMode();

	TWeakObjectPtr<AMaidCharacter> OwnerMaid;
	TWeakObjectPtr<UCharacterMovementComponent> MovementComponent;

	float DashTimeRemaining = 0.f;
	float DashCooldownRemaining = 0.f;
	bool bIsDashing = false;
	bool bSavedOrientRotationToMovement = true;
	float SavedAnimRootMotionScale = 1.f;
	ERootMotionMode::Type SavedRootMotionMode = ERootMotionMode::RootMotionFromMontagesOnly;
	bool bRootMotionModeOverridden = false;
};
