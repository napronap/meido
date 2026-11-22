// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Components/ActorComponent.h"
#include "DataAssets/AttackComboData.h"
#include "AttackComponent.generated.h"

class ACharacter;
class UAnimSequenceBase;

// As it is right now, this AC only works for the maids attacks as it relies on hitboxes mounted on sockets (from the DA)
// could be expanded to take an actor to where to attach the box to I guess
//
//     flow of an attack
//		    Input
//	 		  ↓
//   AMaidPlayerController (relay input)
//		  	  ↓
//	    AMaidCharacter (checks other components/conditions that are not related to this AC)
//			  ↓
//	   UAttackComponent	(checks only internal conditions)
//			  ↓		(if connects)
//	AMaidCharacter::TakeDamage (resolves damage through UE api)
//
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MEIDO_5_6_API UAttackComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAttackComponent();
	
	// starts the combo, or queues the next step while one is playing (ignored while recovering)
	// the step turns the owner to AimDirection when it starts (zero keeps the current facing)
	void RequestAttack(const FVector& AimDirection);

	// true from the first step until the last played step's montage ends (recovery included)
	bool IsAttacking() const { return CurrentStep != INDEX_NONE; }

	// called by the combo notifies with the montage that fired them
	void OpenComboHitWindow(const UAnimSequenceBase* Animation);
	// called by the check combo notify with the montage that fired them
	void CheckCombo(const UAnimSequenceBase* Animation);
	// called by the recovery end notify with the montage that fired them
	void EndRecovery(const UAnimSequenceBase* Animation);
	
	// starts tracing every frame until CloseHitWindow
	void OpenHitWindow(const FAttackHit& Hit);
	void CloseHitWindow();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;

private:
	void PlayStep(int32 StepIndex);
	void HandleStepMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	// need to check if the animation requesting anything from this component is the currently playing one
	// there's .3 s (blend time) in which other animation than the one requesting might be playing
	bool IsCurrentStep(const UAnimSequenceBase* Animation) const;

	void PerformHitTrace();
	bool ShouldIgnoreHitActor(const AActor* HitActor) const;

	UPROPERTY()
	TObjectPtr<ACharacter> OwnerCharacter;

	UPROPERTY(EditAnywhere, Category="Attack")
	TObjectPtr<UAttackComboData> ComboData;

	UPROPERTY(EditAnywhere, Category="Attack|HitStop", meta=(ClampMin="0.0", ClampMax="0.2"))
	float LightHitStopDuration = 0.04f;

	UPROPERTY(EditAnywhere, Category="Attack|HitStop", meta=(ClampMin="0.0", ClampMax="0.3"))
	float HeavyHitStopDuration = 0.08f;

	// combo
	int32 CurrentStep = INDEX_NONE;
	bool bAttackQueued = false;
	bool bRecovering = false;
	// of the step being started or queued
	FVector AimDirection = FVector::ZeroVector;
	FOnMontageBlendingOutStarted HandleStepMontageBlendingOut;

	// hit window
	FAttackHit CurrentHit;
	// where each socket was last frame (same index as CurrentHit.Sockets)
	TArray<FVector> PreviousSocketLocations;
	TSet<TWeakObjectPtr<AActor>> HitActorsThisWindow;
};
