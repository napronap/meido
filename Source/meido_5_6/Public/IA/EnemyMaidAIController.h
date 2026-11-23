#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyMaidAIController.generated.h"

// moved all the behaviors to bp for easier iteration and honestly there's no reason to have AI stuff here anyways
// this still exists because GameModes and Die from MaidCharacter either cast to this class or call SetAIEnabled directly
UCLASS()
class MEIDO_5_6_API AEnemyMaidAIController : public AAIController
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="AI")
	void SetAIEnabled(bool bEnabled);
};
