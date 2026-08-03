// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorComponents/HealthComponent.h"
#include "GameFramework/Actor.h"

UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = FMath::Clamp(MaxHealth, 0.f, MaxHealth);
	bIsDead = CurrentHealth <= 0.f;

	// UE path: ApplyDamage → TakeDamage → OnTakeAnyDamage → ApplyCombatDamage
	if (AActor* OwnerActor = GetOwner())
	{
		OwnerActor->OnTakeAnyDamage.AddDynamic(this, &UHealthComponent::HandleOwnerTakeAnyDamage);
	}
}

void UHealthComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (AActor* OwnerActor = GetOwner())
	{
		OwnerActor->OnTakeAnyDamage.RemoveDynamic(this, &UHealthComponent::HandleOwnerTakeAnyDamage);
	}

	Super::EndPlay(EndPlayReason);
}

float UHealthComponent::ApplyCombatDamage(const FCombatDamageRequest& Request)
{
	if (bIsDead || Request.Amount <= 0.f)
	{
		return 0.f;
	}

	const float PreviousHealth = CurrentHealth;
	CurrentHealth = FMath::Clamp(CurrentHealth - Request.Amount, 0.f, MaxHealth);
	const float AppliedDamage = PreviousHealth - CurrentHealth;

	if (AppliedDamage <= 0.f)
	{
		return 0.f;
	}

	AActor* Causer = Request.DamageCauser.Get();
	AController* Instigator = Request.Instigator.Get();

	OnDamageTaken.Broadcast(this, AppliedDamage, CurrentHealth, Causer, Instigator);
	OnHealthChanged.Broadcast(this, CurrentHealth, MaxHealth, -AppliedDamage);

	if (CurrentHealth <= 0.f && !bIsDead)
	{
		bIsDead = true;
		OnHealthDepleted.Broadcast(this, Causer);
	}

	return AppliedDamage;
}

void UHealthComponent::Heal(const float Amount)
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

void UHealthComponent::ResetToFullHealth()
{
	const float PreviousHealth = CurrentHealth;
	CurrentHealth = FMath::Clamp(MaxHealth, 0.f, MaxHealth);
	bIsDead = CurrentHealth <= 0.f;

	const float Delta = CurrentHealth - PreviousHealth;
	OnHealthChanged.Broadcast(this, CurrentHealth, MaxHealth, Delta);
}

void UHealthComponent::HandleOwnerTakeAnyDamage(
	AActor* DamagedActor,
	const float Damage,
	const UDamageType* DamageType,
	AController* InstigatedBy,
	AActor* DamageCauser
)
{
	(void)DamagedActor;
	(void)DamageType;

	FCombatDamageRequest Request;
	Request.Amount = Damage;
	Request.DamageCauser = DamageCauser;
	Request.Instigator = InstigatedBy;
	ApplyCombatDamage(Request);
}
