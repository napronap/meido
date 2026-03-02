// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "MaidCharacter.h"
#include "PlayerMaidCharacter.generated.h"

class UInputAction;
class USpringArmComponent;
class UCameraComponent;

UCLASS()
class MEIDO_5_6_API APlayerMaidCharacter : public AMaidCharacter
{
	GENERATED_BODY()

public:
	APlayerMaidCharacter();

protected:
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void Tick(float DeltaTime) override;

	/*
	 * Input Actions
	 */
	UPROPERTY(EditAnywhere, Category="Input|Movement")
	UInputAction* MovementAction;

	UPROPERTY(EditAnywhere, Category="Input|Movement")
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, Category="Input|Movement")
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, Category="Input|Combat")
	UInputAction* ComboAttackAction;

	UPROPERTY(EditDefaultsOnly, Category="Input|MeiDou")
	UInputAction* MeiDouMAction;

	UPROPERTY(EditDefaultsOnly, Category="Input|MeiDou")
	UInputAction* MeiDouKAction;

	UPROPERTY(EditDefaultsOnly, Category="Input|MeiDou")
	UInputAction* MeiDouNAction;

	UPROPERTY(EditDefaultsOnly, Category="Input|LockOn")
	UInputAction* LockOnInputAction;

	UPROPERTY(EditDefaultsOnly, Category="Input|LockOn")
	UInputAction* ChangeLockOnInputAction;

	/*
	 * Input callbacks
	 */
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

	void JumpPressed();
	void JumpReleased();

	void ComboAttackPressed();

	void InputMeiDouM();
	void InputMeiDouK();
	void InputMeiDouN();

	void ToggleLockOn();
	void OnLockOnSwitch(const FInputActionValue& Value);
	void OnLockOnSwitchReleased(const FInputActionValue& Value);
	void ApplyLockOnMovementMode(bool bLockOnActive);
	void UpdateAnimLockOnState(bool bLockOnActive);

	/*
	 * Camera
	 */
	UPROPERTY(VisibleAnywhere)
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere)
	UCameraComponent* ViewCamera;

	void RotateCameraToTarget(AActor* Target, float DeltaTime);

	bool bWasLockOnActive = false;

	// How fast lock-on camera yaw follows target. Lower = smoother/slower.
	UPROPERTY(EditAnywhere, Category="Camera|LockOn", meta=(ClampMin="0.1", ClampMax="30.0"))
	float LockOnCameraYawInterpSpeed = 8.f;
};
