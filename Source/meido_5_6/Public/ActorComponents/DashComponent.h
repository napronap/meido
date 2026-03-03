// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DashComponent.generated.h"

class AMaidCharacter;
class UCharacterMovementComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MEIDO_5_6_API UDashComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UDashComponent();

	UFUNCTION(BlueprintCallable, Category="Dash")
	bool TryDash(const FVector2D& MoveInput, const FRotator& ControlRotation, bool bLockOnActive);

	UFUNCTION(BlueprintCallable, Category="Dash")
	void CancelDash();

	UFUNCTION(BlueprintPure, Category="Dash")
	bool IsDashing() const { return bIsDashing; }

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	UPROPERTY(EditAnywhere, Category="Dash", meta=(ClampMin="200.0", UIMin="200.0"))
	float DashSpeed = 1400.f;

	UPROPERTY(EditAnywhere, Category="Dash", meta=(ClampMin="0.01", UIMin="0.01"))
	float DashDuration = 0.22f;

	UPROPERTY(EditAnywhere, Category="Dash", meta=(ClampMin="0.0", UIMin="0.0"))
	float DashCooldown = 0.35f;

	UPROPERTY(EditAnywhere, Category="Dash", meta=(ClampMin="0.0", ClampMax="1.0"))
	float MinDirectionInput = 0.15f;

private:
	bool CanDash() const;
	FVector ComputeDashWorldDirection(const FVector2D& MoveInput, const FRotator& ControlRotation) const;
	FVector2D ComputeDashAnimDirection(const FVector& WorldDirection, const FRotator& ControlRotation) const;
	void StartDash(const FVector& InDashDirection, const FVector2D& AnimDirection);
	void EndDash();
	void UpdateDashAnimState(bool bDashing, const FVector2D& AnimDirection) const;

	TWeakObjectPtr<AMaidCharacter> OwnerMaid;
	TWeakObjectPtr<UCharacterMovementComponent> MovementComponent;

	FVector DashDirection = FVector::ForwardVector;
	float DashTimeRemaining = 0.f;
	float DashCooldownRemaining = 0.f;
	bool bIsDashing = false;
};
