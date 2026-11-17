#pragma once

#include "CoreMinimal.h"

class AMaidCharacter;
class AAIController;

/** Shared path cache and input steering for approach and hold BT tasks. */
namespace MeidoAIMovement
{
	FVector ResolveDirection(AMaidCharacter* InMaid, const FVector& InDestination,
		float InRepathInterval, float InPointAcceptanceRadius, float& InOutNextRepathTime,
		int32& InOutPathPointIndex, TArray<FVector>& InOutPathPoints);

	void ApplyInput(AMaidCharacter* InMaid, AAIController* InController,
		const FVector& InWorldInput, const FVector& InFacingLocation);
}
