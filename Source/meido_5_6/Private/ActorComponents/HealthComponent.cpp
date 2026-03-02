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

	if (AActor* OwnerActor = GetOwner())
	{
		OwnerActor->OnTakeAnyDamage.AddDynamic(this, &UHealthComponent::HandleOwnerTakeAnyDamage);
	}
}

void UHealthComponent::Heal(float Amount)
{
	if (Amount <= 0.f || bIsDead)
	{
		return;
	}

	const float PreviousHealth = CurrentHealth;
	CurrentHealth = FMath::Clamp(CurrentHealth + Amount, 0.f, MaxHealth);
	const float Delta = CurrentHealth - PreviousHealth;

	if (Delta > 0.f)
	{
		OnHealthChanged.Broadcast(this, CurrentHealth, MaxHealth, Delta);
	}
}

void UHealthComponent::HandleOwnerTakeAnyDamage(
	AActor* DamagedActor,
	const float Damage,
	const UDamageType* DamageType,
	AController* InstigatedBy,
	AActor* DamageCauser
)
{
	if (!DamagedActor || bIsDead || Damage <= 0.f)
	{
		return;
	}

	const float PreviousHealth = CurrentHealth;
	CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0.f, MaxHealth);
	const float AppliedDamage = PreviousHealth - CurrentHealth;

	if (AppliedDamage <= 0.f)
	{
		return;
	}

	OnDamageTaken.Broadcast(this, AppliedDamage, CurrentHealth, DamageCauser, InstigatedBy);
	OnHealthChanged.Broadcast(this, CurrentHealth, MaxHealth, -AppliedDamage);

	if (CurrentHealth <= 0.f)
	{
		bIsDead = true;
		OnHealthDepleted.Broadcast(this, DamageCauser);
	}
}

