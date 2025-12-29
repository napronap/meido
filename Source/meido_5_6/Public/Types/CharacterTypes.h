#pragma once

UENUM(BlueprintType)
enum class ECharacterState : uint8
{
	ECS_Idle UMETA(DisplayName = "Idle"),
	ECS_Attacking UMETA(DisplayName = "Attacking"),
	ECS_Recovering UMETA(DisplayName = "Recovering"),
};