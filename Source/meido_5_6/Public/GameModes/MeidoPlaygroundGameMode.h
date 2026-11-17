#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MeidoPlaygroundGameMode.generated.h"

/** Free gameplay: no forced tutorial, wave or result flow. Set BP pawn/PC defaults in the editor. */
UCLASS()
class MEIDO_5_6_API AMeidoPlaygroundGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMeidoPlaygroundGameMode();
};
