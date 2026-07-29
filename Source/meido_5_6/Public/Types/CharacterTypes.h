#pragma once

/**
 * Legacy monolithic character posture enum.
 * Runtime C++ no longer uses this after CP0.3 (CharacterStateComponent + StateTypes.h).
 * Kept only so old Blueprint pins / assets do not hard-break; safe to delete after BP audit.
 * See docs/POST_REFACTOR_NOTES.md and docs/code-units/ECharacterState.md.
 */
UENUM(BlueprintType)
enum class ECharacterState : uint8
{
	ECS_Idle UMETA(DisplayName = "Idle"),
	ECS_Attacking UMETA(DisplayName = "Attacking"),
	ECS_Recovering UMETA(DisplayName = "Recovering"),
	ECS_Jumping UMETA(DisplayName = "Jumping"),
	ECS_Dashing UMETA(DisplayName = "Dashing"),
	ECS_MeiDouActive UMETA(DisplayName = "MeiDouActive"),
	ECS_MeiDouFailed UMETA(DisplayName = "MeiDouFailed"),
};
