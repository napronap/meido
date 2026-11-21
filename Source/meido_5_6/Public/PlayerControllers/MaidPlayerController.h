// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "MaidPlayerController.generated.h"

class APlayerMaidCharacter;
class UInputMappingContext;
class UInputAction;

// All the player input: routes each action to the maid's API (the maid gates it, its components run it)
// Also owns the camera rotation: look input, and the yaw following the lock-on target
UCLASS()
class MEIDO_5_6_API AMaidPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override;

	// before Super applies the look input, while locked on the yaw turns toward the target
	virtual void UpdateRotation(float DeltaTime) override;

	// only using one IMC for now
	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputMappingContext> DefaultIMC;

	UPROPERTY(EditDefaultsOnly, Category="Input|Movement")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, Category="Input|Movement")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, Category="Input|Movement")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditDefaultsOnly, Category="Input|Movement")
	TObjectPtr<UInputAction> DashAction;

	UPROPERTY(EditDefaultsOnly, Category="Input|Combat")
	TObjectPtr<UInputAction> AttackAction;

	UPROPERTY(EditDefaultsOnly, Category="Input|MeiDou")
	TObjectPtr<UInputAction> MeiDouMoeAction;

	UPROPERTY(EditDefaultsOnly, Category="Input|MeiDou")
	TObjectPtr<UInputAction> MeiDouKyunAction;

	UPROPERTY(EditDefaultsOnly, Category="Input|MeiDou")
	TObjectPtr<UInputAction> MeiDouNyanAction;

	UPROPERTY(EditDefaultsOnly, Category="Input|LockOn")
	TObjectPtr<UInputAction> LockOnAction;

	UPROPERTY(EditDefaultsOnly, Category="Input|LockOn")
	TObjectPtr<UInputAction> SwitchLockOnAction;

	// needs bTriggerWhenPaused on the asset to unpause
	UPROPERTY(EditDefaultsOnly, Category="Input|Flow")
	TObjectPtr<UInputAction> PauseAction;

	// freezes/unfreezes every enemy
	UPROPERTY(EditDefaultsOnly, Category="Input|Debug")
	TObjectPtr<UInputAction> DebugToggleAIAction;

	// how fast the camera yaw follows the lockon target
	UPROPERTY(EditDefaultsOnly, Category="Camera", meta=(ClampMin="0.1", ClampMax="30.0"))
	float LockOnYawInterpSpeed = 8.f;

private:
	void Move(const FInputActionValue& Value);
	// needed so that dash doesn't fire on an unwanted direction when letting go of the stick/keys
	void StopMove();
	void Look(const FInputActionValue& Value);
	void Jump();
	void StopJumping();
	void Dash();
	void Attack();
	void MeiDouMoe();
	void MeiDouKyun();
	void MeiDouNyan();
	void ToggleLockOn();
	void SwitchLockOnTarget(const FInputActionValue& Value);
	void TogglePause();
	void ToggleDebugEnemyAI();

	UPROPERTY()
	TObjectPtr<APlayerMaidCharacter> PlayerMaid;

	// last move input, the dash goes that way
	FVector2D MoveInput = FVector2D::ZeroVector;
};
