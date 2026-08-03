#pragma once

#include "CoreMinimal.h"
#include "CombatTeamTypes.generated.h"

/** Combat affiliation for friendly-fire / filters (not a full faction system). */
UENUM(BlueprintType)
enum class ECombatTeam : uint8
{
	Neutral UMETA(DisplayName = "Neutral"),
	Player UMETA(DisplayName = "Player"),
	Enemy UMETA(DisplayName = "Enemy")
};
