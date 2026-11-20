// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorComponents/HealthComponent.h"

// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


// Called when the game starts
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = FMath::Clamp(MaxHealth, 0.f, MaxHealth);
	bIsDead = CurrentHealth <= 0.f;
}

void UHealthComponent::ResetToFullHealth()
{
	CurrentHealth = MaxHealth;
	bIsDead = false;
	
	OnHealthChanged.Broadcast(this, CurrentHealth, MaxHealth, MaxHealth);
}

float UHealthComponent::ApplyDamage(const float Damage, AActor* DamageCauser)
{
	const float AppliedDamage = FMath::Min(Damage, CurrentHealth);
	CurrentHealth -= AppliedDamage;

	OnHealthChanged.Broadcast(this, CurrentHealth, MaxHealth, -AppliedDamage);

	if (CurrentHealth <= 0.f)
	{
		bIsDead = true;
		OnHealthDepleted.Broadcast(this, DamageCauser);
	}

	return AppliedDamage;
}

