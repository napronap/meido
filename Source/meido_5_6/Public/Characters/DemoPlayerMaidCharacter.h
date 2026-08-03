// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "MaidCharacter.h"
#include "Interfaces/CombatTeamSource.h"
#include "DemoPlayerMaidCharacter.generated.h"

class UInputAction;
class USpringArmComponent;
class UCameraComponent;

UCLASS()
class MEIDO_5_6_API ADemoPlayerMaidCharacter : public AMaidCharacter, public ICombatTeamSource
{
	GENERATED_BODY()

public:
	ADemoPlayerMaidCharacter();

	virtual ECombatTeam GetCombatTeam_Implementation() const override;

	UFUNCTION(BlueprintCallable, Category="Camera|Menu")
	void ApplyMenuCameraPose();

	UFUNCTION(BlueprintCallable, Category="Camera|Menu")
	void StartMenuCameraTransitionToGameplay();

	UFUNCTION(BlueprintPure, Category="Camera|Menu")
	bool IsMenuCameraTransitionActive() const { return bMenuCameraTransitionActive; }

	UFUNCTION(BlueprintCallable, Category="Camera|Win")
	void StartWinSequenceCameraTransition();

	UFUNCTION(BlueprintPure, Category="Camera|Win")
	bool IsWinSequenceActive() const { return bWinSequenceActive; }

	UFUNCTION(BlueprintCallable, Category="Camera|Lose")
	void StartLoseSequenceCameraTransition();

	UFUNCTION(BlueprintPure, Category="Camera|Lose")
	bool IsLoseSequenceActive() const { return bLoseSequenceActive; }

	virtual void ResetForFlowRestart() override;

protected:
	virtual void BeginPlay() override;
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

	UPROPERTY(EditAnywhere, Category="Input|Movement")
	UInputAction* DashAction;

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
	void DashPressed();

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

	// how fast lockon camera yaw follows target
	UPROPERTY(EditAnywhere, Category="Camera|LockOn", meta=(ClampMin="0.1", ClampMax="30.0"))
	float LockOnCameraYawInterpSpeed = 8.f;

	// ALL these variables are here for fast iteration. They are related to the transition between mainmenu/play state
	// will probably remove all this and come up with a proper system in the future
#pragma region
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera|Menu")
	bool bApplyMenuCameraPoseOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera|Menu", meta=(ClampMin="0.0", UIMin="0.0"))
	float MenuArmLength = 120.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera|Menu")
	float MenuPitch = -2.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera|Menu")
	float MenuYawOffset = 180.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera|Menu")
	float MenuHorizontalOffset = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera|Gameplay", meta=(ClampMin="0.0", UIMin="0.0"))
	float GameplayArmLength = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera|Gameplay")
	float GameplayPitch = -10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera|Gameplay")
	float GameplayYawOffset = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera|Gameplay")
	float GameplayHorizontalOffset = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera|Transition", meta=(ClampMin="0.01", UIMin="0.01"))
	float MenuToGameplayTransitionDuration = 1.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera|Transition", meta=(ClampMin="1.0", UIMin="1.0"))
	float MenuToGameplayTransitionExponent = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera|Win")
	UAnimMontage* WinMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera|Lose", meta=(ClampMin="0.0", UIMin="0.0"))
	float LoseArmLength = 420.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera|Lose")
	float LosePitch = -75.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera|Lose")
	float LoseYawOffset = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera|Lose")
	float LoseHorizontalOffset = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera|Lose", meta=(ClampMin="0.01", UIMin="0.01"))
	float LoseTransitionDuration = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera|Lose", meta=(ClampMin="1.0", UIMin="1.0"))
	float LoseTransitionExponent = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera|Lose")
	UAnimMontage* GetUpMontage = nullptr;
#pragma endregion
	
	FVector2D CachedMoveInput = FVector2D::ZeroVector;

private:
	bool bMenuCameraTransitionActive = false;
	float MenuCameraTransitionElapsed = 0.f;
	float MenuCameraTransitionStartArmLength = 0.f;
	float MenuCameraTransitionStartSocketOffsetY = 0.f;
	float MenuCameraTransitionTargetSocketOffsetY = 0.f;
	FRotator MenuCameraTransitionStartRotation = FRotator::ZeroRotator;
	FRotator MenuCameraTransitionTargetRotation = FRotator::ZeroRotator;

	bool bWinSequenceActive = false;
	float WinSequenceElapsed = 0.f;
	float WinSequenceStartArmLength = 0.f;
	float WinSequenceStartSocketOffsetY = 0.f;
	float WinSequenceTargetArmLength = 0.f;
	float WinSequenceTargetSocketOffsetY = 0.f;
	FRotator WinSequenceStartRotation = FRotator::ZeroRotator;
	FRotator WinSequenceTargetRotation = FRotator::ZeroRotator;

	bool bLoseSequenceActive = false;
	float LoseSequenceElapsed = 0.f;
	float LoseSequenceStartArmLength = 0.f;
	float LoseSequenceStartSocketOffsetY = 0.f;
	float LoseSequenceTargetArmLength = 0.f;
	float LoseSequenceTargetSocketOffsetY = 0.f;
	FRotator LoseSequenceStartRotation = FRotator::ZeroRotator;
	FRotator LoseSequenceTargetRotation = FRotator::ZeroRotator;

	void UpdateMenuCameraTransition(float DeltaTime);
	void UpdateWinSequenceTransition(float DeltaTime);
	void UpdateLoseSequenceTransition(float DeltaTime);
};
