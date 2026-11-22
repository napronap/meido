// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LockOnComponent.generated.h"

class ACharacter;

// Player lockon: locks the on screen targetable closest to the screen center and switches left/right between
// the onscreen ones
// while locked the character strafes facing away from the camera, and AMaidPlayerController turns the camera yaw to the target
// the target shows its own marker (ITargetable::SetLockedOn)
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

	bool IsLockedOn() const { return CurrentTarget != nullptr; }

	AActor* GetCurrentTarget() const { return CurrentTarget; }

	// only ticks while locked on: retargets when the target is lost
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

	// moves the marker from the old target to the new one (either can be null)
	void SetTarget(AActor* NewTarget);

	// owner rotation mode, ABP flag and tick
	void ApplyLockOnState(bool bLockedOn);

	UPROPERTY(EditAnywhere)
	float SearchRadius = 1200.f;

	UPROPERTY()
	TObjectPtr<ACharacter> OwnerCharacter;

	UPROPERTY()
	TObjectPtr<AActor> CurrentTarget;
};
