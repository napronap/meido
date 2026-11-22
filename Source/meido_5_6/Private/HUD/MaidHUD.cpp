#include "HUD/MaidHUD.h"
#include "ActorComponents/HealthComponent.h"
#include "Characters/MaidCharacter.h"
#include "HUD/PlayerHUDWidget.h"

void AMaidHUD::BeginPlay()
{
	Super::BeginPlay();

	PlayerHUD = CreateWidget<UPlayerHUDWidget>(GetOwningPlayerController(), PlayerHUDClass);
	PlayerHUD->AddToViewport();

	// the pawn is already possessed, but its health component may not have run BeginPlay yet
	UHealthComponent* HealthComponent = CastChecked<AMaidCharacter>(GetOwningPawn())->GetHealthComponent();
	HealthComponent->OnHealthChanged.AddDynamic(this, &AMaidHUD::HandleHealthChanged);
	PlayerHUD->UpdateHealth(HealthComponent->GetMaxHealth(), HealthComponent->GetMaxHealth());
}

void AMaidHUD::SetGameplayHUDVisible(const bool bVisible) const
{
	// the health bar never takes clicks
	PlayerHUD->SetVisibility(bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
}

void AMaidHUD::HandleHealthChanged(UHealthComponent* HealthComponent, const float CurrentHealth, const float MaxHealth, float Delta)
{
	PlayerHUD->UpdateHealth(CurrentHealth, MaxHealth);
}
