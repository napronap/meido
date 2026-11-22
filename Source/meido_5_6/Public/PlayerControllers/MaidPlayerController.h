// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "MaidPlayerController.generated.h"

class APlayerMaidCharacter;
class UPlayerInputConfig;

// All the player input: routes each action to the maid's API (the maid gates it, its components run it)
// Also owns the camera rotation: look input, and the yaw following the lock-on target
// Works with any game mode; each mode's own input (pause, debug AI toggle) lives in its subclass
// (ADemoPlayerController, ASandboxPlayerController)
UCLASS()
class MEIDO_5_6_API AMaidPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	// subclasses bind their own actions after Super
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override;

	// before Super applies the look input, while locked on the yaw turns toward the target
	virtual void UpdateRotation(float DeltaTime) override;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UPlayerInputConfig> InputConfig;

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

	UPROPERTY()
	TObjectPtr<APlayerMaidCharacter> PlayerMaid;

	// last move input, the dash goes that way
	FVector2D MoveInput = FVector2D::ZeroVector;
};
