#include "GameModes/MeidoPlaygroundGameMode.h"
#include "Characters/PlayerMaidCharacter.h"
#include "PlayerControllers/MaidPlayerController.h"

AMeidoPlaygroundGameMode::AMeidoPlaygroundGameMode()
{
	DefaultPawnClass = APlayerMaidCharacter::StaticClass();
	PlayerControllerClass = AMaidPlayerController::StaticClass();
}
