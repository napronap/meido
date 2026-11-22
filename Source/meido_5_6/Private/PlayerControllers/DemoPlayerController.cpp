#include "PlayerControllers/DemoPlayerController.h"
#include "EnhancedInputComponent.h"
#include "Engine/World.h"
#include "GameModes/DemoCameraManager.h"
#include "GameModes/DemoGameMode.h"

ADemoPlayerController::ADemoPlayerController()
{
	PlayerCameraManagerClass = ADemoCameraManager::StaticClass();
}

void ADemoPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EnhancedInput = CastChecked<UEnhancedInputComponent>(InputComponent);
	EnhancedInput->BindAction(PauseAction, ETriggerEvent::Started, this, &ADemoPlayerController::TogglePause);
	EnhancedInput->BindAction(DebugToggleAIAction, ETriggerEvent::Started, this, &ADemoPlayerController::ToggleDebugEnemyAI);
}

void ADemoPlayerController::TogglePause()
{
	GetWorld()->GetAuthGameMode<ADemoGameMode>()->TogglePause();
}

void ADemoPlayerController::ToggleDebugEnemyAI()
{
	GetWorld()->GetAuthGameMode<ADemoGameMode>()->ToggleDebugEnemyAI();
}
