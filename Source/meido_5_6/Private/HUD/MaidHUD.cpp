#include "HUD/MaidHUD.h"
#include "Blueprint/UserWidget.h"

void AMaidHUD::BeginPlay()
{
	Super::BeginPlay();

	// the pawn is already possessed: the widget binds to its components when it's initialized
	PlayerHUD = CreateWidget(GetOwningPlayerController(), PlayerHUDClass);
	PlayerHUD->AddToViewport();
}

void AMaidHUD::SetGameplayHUDVisible(const bool bVisible) const
{
	PlayerHUD->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}
