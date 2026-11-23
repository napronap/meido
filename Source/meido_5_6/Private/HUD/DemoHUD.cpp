#include "HUD/DemoHUD.h"
#include "Blueprint/UserWidget.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

void ADemoHUD::BeginPlay()
{
	Super::BeginPlay();

	// the GameMode sets its first state in StartPlay after this
	GetWorld()->GetAuthGameMode<ADemoGameMode>()->OnFlowStateChanged.AddDynamic(this, &ADemoHUD::HandleFlowStateChanged);
}

void ADemoHUD::HandleFlowStateChanged(const EFlowState NewState)
{
	SetGameplayHUDVisible(NewState == EFlowState::Playing || NewState == EFlowState::Paused || NewState == EFlowState::Tutorial);

	if (FlowWidget)
	{
		FlowWidget->RemoveFromParent();
		FlowWidget = nullptr;
	}

	APlayerController* PC = GetOwningPlayerController();

	if (const TSubclassOf<UUserWidget>* WidgetClass = FlowWidgets.Find(NewState))
	{
		// above the gameplay HUD
		FlowWidget = CreateWidget<UUserWidget>(PC, *WidgetClass);
		FlowWidget->AddToViewport(100);
		PC->SetShowMouseCursor(true);

		// paused keeps game input so the pause key can close it, the other screens are UI only
		if (NewState == EFlowState::Paused)
		{
			FInputModeGameAndUI InputMode;
			InputMode.SetWidgetToFocus(FlowWidget->TakeWidget());
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			InputMode.SetHideCursorDuringCapture(false);
			PC->SetInputMode(InputMode);
		}
		else
		{
			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(FlowWidget->TakeWidget());
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PC->SetInputMode(InputMode);
		}
	}
	else
	{
		PC->SetShowMouseCursor(false);
		PC->SetInputMode(FInputModeGameOnly());
	}
}
