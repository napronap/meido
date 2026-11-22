#include "PlayerControllers/SandboxPlayerController.h"
#include "EnhancedInputComponent.h"
#include "Engine/World.h"
#include "GameModes/SandboxGameMode.h"

void ASandboxPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EnhancedInput = CastChecked<UEnhancedInputComponent>(InputComponent);
	EnhancedInput->BindAction(DebugToggleAIAction, ETriggerEvent::Started, this, &ASandboxPlayerController::ToggleDebugEnemyAI);
}

void ASandboxPlayerController::ToggleDebugEnemyAI()
{
	GetWorld()->GetAuthGameMode<ASandboxGameMode>()->ToggleDebugEnemyAI();
}
