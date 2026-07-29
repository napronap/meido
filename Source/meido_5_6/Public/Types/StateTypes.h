// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTypes.generated.h"

/** Derived posture for UI/AI/anim high-level queries. Recalculated from slices. */
UENUM(BlueprintType)
enum class ECharacterOverallState : uint8
{
	Idle UMETA(DisplayName = "Idle"),
	Jumping UMETA(DisplayName = "Jumping"),
	Dashing UMETA(DisplayName = "Dashing"),
	Attacking UMETA(DisplayName = "Attacking"),
	MeiDou UMETA(DisplayName = "MeiDou"),
	MeiDouFailed UMETA(DisplayName = "MeiDouFailed"),
	Stagger UMETA(DisplayName = "Stagger"),
	Dead UMETA(DisplayName = "Dead"),
};

/** Melee chain detail (combo driver owns writes). */
UENUM(BlueprintType)
enum class EAttackState : uint8
{
	None UMETA(DisplayName = "None"),
	Starting UMETA(DisplayName = "Starting"),
	InSwing UMETA(DisplayName = "InSwing"),
	ComboWindow UMETA(DisplayName = "ComboWindow"),
	WhiffRecover UMETA(DisplayName = "WhiffRecover"),
};

/** Health action layer (stagger vs dead vs fine). Not HP numbers. */
UENUM(BlueprintType)
enum class EHealthActionState : uint8
{
	Alive UMETA(DisplayName = "Alive"),
	Stagger UMETA(DisplayName = "Stagger"),
	Dead UMETA(DisplayName = "Dead"),
};

/**
 * Locomotion layer (dash / jump-intent hooks).
 *
 * Falling is not being tracked because:
 * - (x) no call site sets it — only Jump (Jump/StopJumping) and Dash are written today;
 * - (y) air gates already use CharacterMovement->IsFalling() (combo, MeiDou, dash) — no need to mirror that in State;
 * - (z) polling IsFalling every tick just to fill this enum is waste; if we ever need in-air posture in State,
 *   wire OnMovementModeChanged / Landed (event-driven), not Tick.
 * Kept as a reserved value for that future path (air attack / Overall / anim).
 */
UENUM(BlueprintType)
enum class ELocomotionState : uint8
{
	Grounded UMETA(DisplayName = "Grounded"),
	Jump UMETA(DisplayName = "Jump"),
	Dash UMETA(DisplayName = "Dash"),
	Falling UMETA(DisplayName = "Falling"),
};

/**
 * Coarse MeiDou layer for gates / Overall.
 * Pose vs Result detail stays on UMeiDouComponent.
 */
UENUM(BlueprintType)
enum class EMeiDouLayerState : uint8
{
	Idle UMETA(DisplayName = "Idle"),
	Active UMETA(DisplayName = "Active"),
	Failed UMETA(DisplayName = "Failed"),
};
