// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LockOnComponent.generated.h"

class ACharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnLockOnChanged,
	AActor*, NewTarget,
	bool, bSuccess
);

// Player lockon: locks the on screen targetable closest to the screen center and switches left/right between
// the onscreen ones
// While locked it keeps the camera yaw on the target and the character strafes facing it
// TODO: move the lock to the PC, this component should only broadcast its change
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MEIDO_5_6_API ULockOnComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULockOnComponent();

	UFUNCTION(BlueprintCallable)
	bool TryLockOn();

	UFUNCTION(BlueprintCallable)
	void ClearLockOn();

	// +1 next on screen target to the right, -1 to the left
	void SwitchTarget(int32 Direction);

	// impure on purpose: BP_PlayerMaid calls both inside an exec chain (lockon marker)
	// TODO: remove
	UFUNCTION(BlueprintCallable, BlueprintPure=false)
	bool IsLockedOn() const { return CurrentTarget != nullptr; }

	UFUNCTION(BlueprintCallable, BlueprintPure=false)
	AActor* GetCurrentTarget() const { return CurrentTarget; }

	UPROPERTY(BlueprintAssignable)
	FOnLockOnChanged OnLockOnChanged;

	// only ticks while locked on: retargets when the target is lost and keeps the camera yaw on it
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;

private:
	struct FScreenTarget
	{
		AActor* Actor;
		// offset from screen center
		FVector2D ScreenOffset;
	};

	// targetables in SearchRadius that are inside the viewport
	TArray<FScreenTarget> GetOnScreenTargets() const;

	static bool IsTargetable(AActor* Actor);

	// owner rotation mode, ABP flag and tick
	void ApplyLockOnState(bool bLockedOn);

	UPROPERTY(EditAnywhere)
	float SearchRadius = 1200.f;

	// how fast the camera yaw follows the target
	UPROPERTY(EditAnywhere, meta=(ClampMin="0.1", ClampMax="30.0"))
	float CameraYawInterpSpeed = 8.f;

	UPROPERTY()
	TObjectPtr<ACharacter> OwnerCharacter;

	UPROPERTY()
	TObjectPtr<AActor> CurrentTarget;
};
