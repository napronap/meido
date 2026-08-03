// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

class AController;
class AActor;
class UDamageType;
class UHealthComponent;

/**
 * Combat damage request (CP1.2).
 * Prefer Health->ApplyCombatDamage over raw ApplyDamage when calling from C++.
 * UE ApplyDamage → TakeDamage → OnTakeAnyDamage still ends here.
 */
USTRUCT(BlueprintType)
struct FCombatDamageRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float Amount = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	TObjectPtr<AActor> DamageCauser = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	TObjectPtr<AController> Instigator = nullptr;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(
	FOnHealthChanged,
	UHealthComponent*, HealthComponent,
	float, CurrentHealth,
	float, MaxHealth,
	float, Delta
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(
	FOnHealthDamageTaken,
	UHealthComponent*, HealthComponent,
	float, Damage,
	float, CurrentHealth,
	AActor*, DamageCauser,
	AController*, InstigatedBy
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnHealthDepleted,
	UHealthComponent*, HealthComponent,
	AActor*, DamageCauser
);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MEIDO_5_6_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHealthComponent();

	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnHealthChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnHealthDamageTaken OnDamageTaken;

	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnHealthDepleted OnHealthDepleted;

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetCurrentHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetMaxHealth() const { return MaxHealth; }

	/** Primary dead flag for combat (character presentation may mirror with bHasDied). */
	UFUNCTION(BlueprintPure, Category = "Health")
	bool IsDead() const { return bIsDead; }

	/**
	 * Apply combat damage to the HP pool and broadcast domain events.
	 * Does not know character gates (i-frames, MeiDouFailed) — those stay on the pawn TakeDamage.
	 * @return Actual HP removed (0 if dead / no effect).
	 */
	UFUNCTION(BlueprintCallable, Category = "Health")
	float ApplyCombatDamage(const FCombatDamageRequest& Request);

	UFUNCTION(BlueprintCallable, Category = "Health")
	void Heal(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Health")
	void ResetToFullHealth();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health", meta = (AllowPrivateAccess = "true"))
	float MaxHealth = 100.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health", meta = (AllowPrivateAccess = "true"))
	float CurrentHealth = 100.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health", meta = (AllowPrivateAccess = "true"))
	bool bIsDead = false;

	UFUNCTION()
	void HandleOwnerTakeAnyDamage(
		AActor* DamagedActor,
		float Damage,
		const UDamageType* DamageType,
		AController* InstigatedBy,
		AActor* DamageCauser
	);
};
