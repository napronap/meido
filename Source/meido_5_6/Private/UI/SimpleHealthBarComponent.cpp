// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/SimpleHealthBarComponent.h"

#include "ActorComponents/HealthComponent.h"
#include "Components/ProgressBar.h"
#include "UI/SimpleHealthBar.h"

USimpleHealthBarComponent::USimpleHealthBarComponent()
{
	// No placement/size/space here — author sets those on the component in BP.
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetGenerateOverlapEvents(false);
}

void USimpleHealthBarComponent::BeginPlay()
{
	if (HealthBarClass)
	{
		SetWidgetClass(HealthBarClass);
	}

	Super::BeginPlay();

	HealthBarWidget = Cast<USimpleHealthBar>(GetUserWidgetObject());
	if (!HealthBarWidget)
	{
		InitWidget();
		HealthBarWidget = Cast<USimpleHealthBar>(GetUserWidgetObject());
	}

	BindToOwnerHealth();
}

void USimpleHealthBarComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindHealth();
	HealthBarWidget = nullptr;
	Super::EndPlay(EndPlayReason);
}

void USimpleHealthBarComponent::SetHealthPercent(const float Percent)
{
	if (!HealthBarWidget)
	{
		return;
	}

	if (UProgressBar* const Bar = HealthBarWidget->HealthBar.Get())
	{
		Bar->SetPercent(FMath::Clamp(Percent, 0.f, 1.f));
	}
}

void USimpleHealthBarComponent::BindToOwnerHealth()
{
	UnbindHealth();

	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	BoundHealth = OwnerActor->FindComponentByClass<UHealthComponent>();
	if (!BoundHealth)
	{
		return;
	}

	BoundHealth->OnHealthChanged.AddDynamic(this, &USimpleHealthBarComponent::HandleHealthChanged);
	ApplyPercentFromHealth(BoundHealth->GetCurrentHealth(), BoundHealth->GetMaxHealth());
}

void USimpleHealthBarComponent::UnbindHealth()
{
	if (BoundHealth)
	{
		BoundHealth->OnHealthChanged.RemoveDynamic(this, &USimpleHealthBarComponent::HandleHealthChanged);
		BoundHealth = nullptr;
	}
}

void USimpleHealthBarComponent::ApplyPercentFromHealth(const float CurrentHealth, const float MaxHealth)
{
	const float SafeMax = FMath::Max(MaxHealth, KINDA_SMALL_NUMBER);
	SetHealthPercent(CurrentHealth / SafeMax);
}

void USimpleHealthBarComponent::HandleHealthChanged(
	UHealthComponent* /*HealthComponent*/,
	const float CurrentHealth,
	const float MaxHealth,
	const float /*Delta*/
)
{
	ApplyPercentFromHealth(CurrentHealth, MaxHealth);

	if (CurrentHealth <= 0.f)
	{
		UnbindHealth();
	}
}
