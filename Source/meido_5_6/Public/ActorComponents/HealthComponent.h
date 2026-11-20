// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

class AActor;
class UHealthComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(
	FOnHealthChanged,
	UHealthComponent*, HealthComponent,
	float, CurrentHealth,
	float, MaxHealth,
	float, Delta
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnHealthDepleted,
	UHealthComponent*, HealthComponent,
	AActor*, DamageCauser
);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MEIDO_5_6_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHealthComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// consumed by HUD and game flow
	UPROPERTY(BlueprintAssignable, Category="Health")
	FOnHealthChanged OnHealthChanged;

	// consumed by HUD and game flow
	UPROPERTY(BlueprintAssignable, Category="Health")
	FOnHealthDepleted OnHealthDepleted;

	// called by the owner's TakeDamage, returns the health actually removed
	float ApplyDamage(float Damage, AActor* DamageCauser);

	UFUNCTION(BlueprintPure, Category="Health")
	float GetCurrentHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintPure, Category="Health")
	float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintPure, Category="Health")
	bool IsDead() const { return bIsDead; }

	UFUNCTION(BlueprintCallable, Category="Health")
	void ResetToFullHealth();

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Health", meta=(AllowPrivateAccess="true"))
	float MaxHealth = 100.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Health", meta=(AllowPrivateAccess="true"))
	float CurrentHealth = 100.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Health", meta=(AllowPrivateAccess="true"))
	bool bIsDead = false;
};
