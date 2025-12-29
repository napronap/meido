#pragma once

UENUM(BlueprintType)
enum class ECharacterCombatState : uint8
{
	ECCS_Idle UMETA(DisplayName = "Idle"),
	ECCS_Attacking UMETA(DisplayName = "Attacking"),
	ECCS_Recovering UMETA(DisplayName = "Recovering"),
};
