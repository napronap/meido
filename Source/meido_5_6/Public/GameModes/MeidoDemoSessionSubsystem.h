#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MeidoDemoSessionSubsystem.generated.h"

/** Tutorial memory for this application/PIE session, including map reloads. */
UCLASS()
class MEIDO_5_6_API UMeidoDemoSessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	bool HasCompletedTutorial() const { return bCompletedTutorial; }
	void MarkTutorialCompleted() { bCompletedTutorial = true; }

private:
	bool bCompletedTutorial = false;
};
