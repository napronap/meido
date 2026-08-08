// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MaidCameraManagerComponent.generated.h"

class AActor;
class UCameraComponent;
class ULockOnComponent;
class USpringArmComponent;

UENUM(BlueprintType)
enum class EMaidCameraProfile : uint8
{
	Free UMETA(DisplayName = "Free"),
	LockOn UMETA(DisplayName = "LockOn")
	// Menu / Win / Lose → V2 (stay on DemoPlayer for now)
};

/**
 * Gameplay camera owner (CP2.2 V1): Free lag + LockOn follow + spring-arm collision policy.
 * Menu/Win/Lose remain on DemoPlayer until V2.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MEIDO_5_6_API UMaidCameraManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMaidCameraManagerComponent();

	UFUNCTION(BlueprintCallable, Category = "Camera")
	void Initialize(USpringArmComponent* InBoom, UCameraComponent* InCamera);

	UFUNCTION(BlueprintCallable, Category = "Camera")
	void SetProfile(EMaidCameraProfile NewProfile);

	UFUNCTION(BlueprintPure, Category = "Camera")
	EMaidCameraProfile GetProfile() const { return ActiveProfile; }

	UFUNCTION(BlueprintCallable, Category = "Camera")
	void BindLockOn(ULockOnComponent* InLockOn);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction
	) override;

	// --- Free profile (pivot / rotation lag; no shoulder) ---

	UPROPERTY(EditAnywhere, Category = "Camera|Free", meta = (ClampMin = "0.0"))
	float FreeArmLength = 300.f;

	UPROPERTY(EditAnywhere, Category = "Camera|Free")
	bool bFreeEnableRotationLag = true;

	UPROPERTY(EditAnywhere, Category = "Camera|Free", meta = (ClampMin = "0.1", ClampMax = "50.0", EditCondition = "bFreeEnableRotationLag"))
	float FreeRotationLagSpeed = 12.f;

	UPROPERTY(EditAnywhere, Category = "Camera|Free")
	bool bFreeEnablePositionLag = true;

	UPROPERTY(EditAnywhere, Category = "Camera|Free", meta = (ClampMin = "0.1", ClampMax = "50.0", EditCondition = "bFreeEnablePositionLag"))
	float FreePositionLagSpeed = 10.f;

	// --- LockOn profile ---

	UPROPERTY(EditAnywhere, Category = "Camera|LockOn", meta = (ClampMin = "0.0"))
	float LockOnArmLength = 300.f;

	/** Yaw follow toward lock target (control rotation). */
	UPROPERTY(EditAnywhere, Category = "Camera|LockOn", meta = (ClampMin = "0.1", ClampMax = "30.0"))
	float LockOnYawInterpSpeed = 8.f;

	/** Usually off so lag does not fight aim. */
	UPROPERTY(EditAnywhere, Category = "Camera|LockOn")
	bool bLockOnEnableRotationLag = false;

	UPROPERTY(EditAnywhere, Category = "Camera|LockOn")
	bool bLockOnEnablePositionLag = true;

	UPROPERTY(EditAnywhere, Category = "Camera|LockOn", meta = (ClampMin = "0.1", ClampMax = "50.0", EditCondition = "bLockOnEnablePositionLag"))
	float LockOnPositionLagSpeed = 14.f;

	// --- Collision policy (V1) ---

	UPROPERTY(EditAnywhere, Category = "Camera|Collision")
	bool bDoCameraCollision = true;

	/** Sphere radius of the spring-arm probe. Smaller = less “poste me come”. */
	UPROPERTY(EditAnywhere, Category = "Camera|Collision", meta = (ClampMin = "1.0", EditCondition = "bDoCameraCollision"))
	float ProbeSize = 8.f;

	/**
	 * Probe channel (default Camera).
	 * Author: set Pawn / thin props to not block ECC_Camera so characters/posts don't eat the arm.
	 * Walls (WorldStatic) should still block.
	 */
	UPROPERTY(EditAnywhere, Category = "Camera|Collision", meta = (EditCondition = "bDoCameraCollision"))
	TEnumAsByte<ECollisionChannel> ProbeChannel = ECC_Camera;

private:
	void ApplyActiveProfileSettings();
	void UpdateLockOnCamera(float DeltaTime);
	void RotateControlYawTowardTarget(AActor* Target, float DeltaTime);

	UFUNCTION()
	void HandleLockOnChanged(AActor* NewTarget, bool bSuccess);

	UPROPERTY(Transient)
	TObjectPtr<USpringArmComponent> CameraBoom = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UCameraComponent> ViewCamera = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<ULockOnComponent> BoundLockOn = nullptr;

	EMaidCameraProfile ActiveProfile = EMaidCameraProfile::Free;
};
