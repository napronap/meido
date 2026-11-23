#include "IA/EnemyMaidAIController.h"
#include "BrainComponent.h"

void AEnemyMaidAIController::SetAIEnabled(const bool bEnabled)
{
	if (bEnabled == BrainComponent->IsRunning())
	{
		return;
	}
	
	if (bEnabled)
	{
		BrainComponent->RestartLogic();
	}
	else
	{
		BrainComponent->StopLogic(TEXT("AI disabled"));
	}
}
