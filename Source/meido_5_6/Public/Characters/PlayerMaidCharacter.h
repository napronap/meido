// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "Characters/MaidCharacter.h"
#include "Interfaces/CombatTeamSource.h"
#include "PlayerMaidCharacter.generated.h"

class UInputAction;
class USpringArmComponent;
class UCameraComponent;
class UMaidCameraManagerComponent;
class UMeiDouPoseBubbleComponent;

/**
 * Playground / product player. Owns player-only UI like MeiDou pose bubble (CP3.5).
 */
UCLASS()
class MEIDO_5_6_API APlayerMaidCharacter : public AMaidCharacter, public ICombatTeamSource
{
	GENERATED_BODY()
	
public:
	APlayerMaidCharacter();

	virtual ECombatTeam GetCombatTeam_Implementation() const override;

	UFUNCTION(BlueprintPure, Category = "MeiDou|UI")
	UMeiDouPoseBubbleComponent* GetPoseBubbleComponent() const { return PoseBubbleComponent; }
	
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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UMaidCameraManagerComponent* CameraManager;

	/** Player-only speech bubble for poses (not on enemies). Assign Bubble Widget Class in BP. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MeiDou|UI")
	TObjectPtr<UMeiDouPoseBubbleComponent> PoseBubbleComponent;

	bool bWasLockOnActive = false;

	FVector2D CachedMoveInput = FVector2D::ZeroVector;
};
