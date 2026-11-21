// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Components/ActorComponent.h"
#include "DashComponent.generated.h"

class ACharacter;
class UAnimMontage;

// The montage is the dash. Root motion moves the character and its blend out ends the dash
// !! all four montages need root motion and auto blend out !!
//
//  free movement: turns to the input and plays Forward
//  lockon: keeps facing the target and picks Forward/Left/Right from the input
//	no input always results in Backward playing
//
// Flow is the same as attacks: input -> AMaidCharacter::DoDash (CanAct, iframes) -> TryDash (internal checks only)
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MEIDO_5_6_API UDashComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDashComponent();

	// MoveInput is the raw stick (X right, Y forward), relative to ControlRotation's yaw
	bool TryDash(const FVector2D& MoveInput, const FRotator& ControlRotation, bool bLockOnActive);

	bool IsDashing() const { return bIsDashing; }

protected:
	virtual void BeginPlay() override;

private:
	UAnimMontage* PickMontage(const FVector& WorldDirection, bool bHasInput, bool bLockOnActive) const;
	void HandleDashMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);

	UPROPERTY(EditAnywhere, Category="Dash|Montages")
	TObjectPtr<UAnimMontage> ForwardMontage;

	UPROPERTY(EditAnywhere, Category="Dash|Montages")
	TObjectPtr<UAnimMontage> BackwardMontage;

	UPROPERTY(EditAnywhere, Category="Dash|Montages")
	TObjectPtr<UAnimMontage> LeftMontage;

	UPROPERTY(EditAnywhere, Category="Dash|Montages")
	TObjectPtr<UAnimMontage> RightMontage;

	// stick deadzone
	UPROPERTY(EditAnywhere, Category="Dash", meta=(ClampMin="0.0", ClampMax="1.0"))
	float MinDirectionInput = 0.15f;

	UPROPERTY()
	TObjectPtr<ACharacter> OwnerCharacter;

	FOnMontageBlendingOutStarted OnDashMontageBlendingOut;
	bool bIsDashing = false;
};
