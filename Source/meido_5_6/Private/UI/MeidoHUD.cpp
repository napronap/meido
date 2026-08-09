// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/MeidoHUD.h"

#include "ActorComponents/HealthComponent.h"
#include "Components/ProgressBar.h"

void UMeidoHUD::BindToHealth(UHealthComponent* Health)
{
	UnbindHealth();

	if (!Health)
	{
		return;
	}

	BoundHealth = Health;
	BoundHealth->OnHealthChanged.AddDynamic(this, &UMeidoHUD::HandleHealthChanged);
	ApplyPercentFromHealth(BoundHealth->GetCurrentHealth(), BoundHealth->GetMaxHealth());
}

void UMeidoHUD::UnbindHealth()
{
	if (BoundHealth)
	{
		BoundHealth->OnHealthChanged.RemoveDynamic(this, &UMeidoHUD::HandleHealthChanged);
		BoundHealth = nullptr;
	}
}

void UMeidoHUD::SetHealthPercent(const float Percent)
{
	if (UProgressBar* const Bar = HealthBar.Get())
	{
		Bar->SetPercent(FMath::Clamp(Percent, 0.f, 1.f));
	}
}

void UMeidoHUD::ApplyPercentFromHealth(const float CurrentHealth, const float MaxHealth)
{
	const float SafeMax = FMath::Max(MaxHealth, KINDA_SMALL_NUMBER);
	SetHealthPercent(CurrentHealth / SafeMax);
}

void UMeidoHUD::HandleHealthChanged(
	UHealthComponent* /*HealthComponent*/,
	const float CurrentHealth,
	const float MaxHealth,
	const float /*Delta*/
)
{
	ApplyPercentFromHealth(CurrentHealth, MaxHealth);
}
