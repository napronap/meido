#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MeiDouResult.generated.h"

// An actor spawned by a MeiDou combo result (omurice bomb and maid beam)
// MeiDouComponent spawns it deferred with the caster as Owner and Instigator and fills Target
// before BeginPlay, and calls OnResultEnded when the result montage is done
UCLASS(Abstract)
class MEIDO_5_6_API AMeiDouResult : public AActor
{
	GENERATED_BODY()

public:
	// the result montage reached its ActionEnd
	UFUNCTION(BlueprintNativeEvent, Category="MeiDou")
	void OnResultEnded();

	// caster's lockon target when the result spawned
	UPROPERTY(BlueprintReadOnly, Category="MeiDou")
	TObjectPtr<AActor> Target;

protected:
	// whether it lives only as long as the result montage (beam) or manages its own lifetime (bomb)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="MeiDou")
	bool bEndsWithResult = false;
};
