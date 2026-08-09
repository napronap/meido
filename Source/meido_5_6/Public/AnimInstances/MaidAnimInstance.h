// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Types/StateTypes.h"
#include "MaidAnimInstance.generated.h"

class ACharacter;

/**
 * Presentation layer for maid AnimBP (CP2.5).
 * Gameplay truth = State / systems; this only exposes props the graph can read.
 *
 * Locomotion names/math match ABP_Manny_Maid EventGraph:
 *   docs/refs/Maid_ABP_Ref
 * Until the ABP drops its BP variables + BlueprintUpdate sets, graph still uses
 * the pack's BP vars; C++ props are the migration target (and for a clean ABP).
 *
 * Event push still OK for ephemeral anim-only data (mirror, dash BS axes).
 */
UCLASS()
class MEIDO_5_6_API UMaidAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	// --- Event / system push ---

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Maid")
	bool bShouldMirror = false;

	UPROPERTY(BlueprintReadOnly, Category = "Maid|Movement")
	bool bIsLockedOn = false;

	UPROPERTY(BlueprintReadOnly, Category = "Maid|Dash")
	bool bIsDashing = false;

	/** Blend-space Y (−1…1). Written by DashComponent on start/end. */
	UPROPERTY(BlueprintReadOnly, Category = "Maid|Dash", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float DashForward = 0.f;

	/** Blend-space X (−1…1). Written by DashComponent on start/end. */
	UPROPERTY(BlueprintReadOnly, Category = "Maid|Dash", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float DashRight = 0.f;

	// --- NativeUpdate: loco contract = ABP_Manny_Maid (see Maid_ABP_Ref) ---

	/** CMC velocity (full 3D). ABP BP var: Velocity. */
	UPROPERTY(BlueprintReadOnly, Category = "Maid|Movement")
	FVector Velocity = FVector::ZeroVector;

	/** Horizontal speed = VSizeXY(Velocity). ABP BP var: GroundSpeed. */
	UPROPERTY(BlueprintReadOnly, Category = "Maid|Movement")
	float GroundSpeed = 0.f;

	/** Dot(Velocity, ActorForward). ABP BP var: ForwardSpeed. */
	UPROPERTY(BlueprintReadOnly, Category = "Maid|Movement")
	float ForwardSpeed = 0.f;

	/** Dot(Velocity, ActorRight). ABP BP var: RightSpeed. */
	UPROPERTY(BlueprintReadOnly, Category = "Maid|Movement")
	float RightSpeed = 0.f;

	/**
	 * True if GroundSpeed > ShouldMoveSpeedThreshold and CMC has non-zero acceleration.
	 * ABP BP var: ShouldMove (threshold 3 in pack graph).
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Maid|Movement")
	bool bShouldMove = false;

	/** CMC IsFalling(). ABP BP var: IsFalling. */
	UPROPERTY(BlueprintReadOnly, Category = "Maid|Movement")
	bool bIsFalling = false;

	/** CMC IsSwimming(). ABP BP var: IsInWater (pack leftover; rarely used in MEIDO). */
	UPROPERTY(BlueprintReadOnly, Category = "Maid|Movement")
	bool bIsInWater = false;

	/** Same threshold as ABP pack EventGraph (GroundSpeed > 3). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maid|Movement", meta = (ClampMin = "0.0"))
	float ShouldMoveSpeedThreshold = 3.f;

	// --- NativeUpdate: basic State copies (not in current pack EventGraph yet) ---

	UPROPERTY(BlueprintReadOnly, Category = "Maid|State")
	ECharacterOverallState OverallState = ECharacterOverallState::Idle;

	UPROPERTY(BlueprintReadOnly, Category = "Maid|State")
	bool bIsDead = false;

	UPROPERTY(BlueprintReadOnly, Category = "Maid|State")
	bool bIsAttacking = false;

	UPROPERTY(BlueprintReadOnly, Category = "Maid|State")
	bool bIsStaggered = false;

	UPROPERTY(BlueprintReadOnly, Category = "Maid|State")
	bool bIsMeiDouActive = false;

private:
	UPROPERTY(Transient)
	TObjectPtr<ACharacter> OwnerCharacter = nullptr;
};
