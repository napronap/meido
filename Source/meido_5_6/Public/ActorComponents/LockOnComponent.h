// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LockOnComponent.generated.h"

class ACharacter;
class APlayerController;
class UUserWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnLockOnChanged,
	AActor*, NewTarget,
	bool, bSuccess
);

/**
 * Targeting owner for lock-on: sphere search, ITargetable filter, screen-center pick,
 * left/right switch, validity refresh. Camera/movement stay on player + camera manager.
 * Optional UI marker consumes the same OnLockOnChanged path (CP2.3).
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MEIDO_5_6_API ULockOnComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULockOnComponent();

	UFUNCTION(BlueprintCallable, Category = "LockOn")
	bool TryLockOn();

	UFUNCTION(BlueprintCallable, Category = "LockOn")
	void ClearLockOn();

	/** Refreshes validity (auto re-pick if current invalid). */
	UFUNCTION(BlueprintCallable, Category = "LockOn")
	bool IsLockedOn();

	UFUNCTION(BlueprintCallable, Category = "LockOn")
	AActor* GetCurrentTarget();

	UFUNCTION(BlueprintCallable, Category = "LockOn")
	void HandleSwitchInput(float AxisValue);

	UFUNCTION(BlueprintCallable, Category = "LockOn")
	void HandleSwitchReleased();

	void SwitchTarget(int32 Direction);

	/** Domain event: acquired/switched (bSuccess+Target) or cleared (null, false). UI marker + camera bind here. */
	UPROPERTY(BlueprintAssignable, Category = "LockOn")
	FOnLockOnChanged OnLockOnChanged;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction
	) override;

	void FindTargets(TArray<AActor*>& OutTargets) const;
	AActor* SelectBestTarget(const TArray<AActor*>& Targets, APlayerController* PC) const;
	bool IsTargetValidForLockOn(const AActor* Target) const;
	bool RefreshCurrentTarget();

	static FVector ResolveTargetWorldLocation(const AActor* Target);
	APlayerController* GetOwnerPlayerController() const;

	void ApplyMarkerForLockState(AActor* NewTarget, bool bSuccess);
	void DestroyMarkerWidget();
	void UpdateMarkerScreenPosition();

	UPROPERTY(Transient)
	TObjectPtr<ACharacter> OwnerCharacter = nullptr;

	UPROPERTY(EditAnywhere, Category = "LockOn", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float SearchRadius = 1200.f;

	/** Object types for sphere overlap (default Pawn). Tune in BP if enemies use another channel. */
	UPROPERTY(EditAnywhere, Category = "LockOn")
	TArray<TEnumAsByte<EObjectTypeQuery>> TargetObjectType;

	/**
	 * Deadzone for stick/axis switch (latch until release or return to neutral).
	 * Very low by design: camera is locked so accidental look shouldn't drive switch.
	 */
	UPROPERTY(EditAnywhere, Category = "LockOn|Switch", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SwitchInputDeadZone = 0.001f;

	// --- Marker (consumes OnLockOnChanged; position tick while locked) ---

	UPROPERTY(EditAnywhere, Category = "LockOn|Marker")
	bool bShowLockOnMarker = true;

	/** Author: usually WBP_LockOnMarker. Null = no widget (delegate still fires for camera/BP). */
	UPROPERTY(EditAnywhere, Category = "LockOn|Marker")
	TSubclassOf<UUserWidget> MarkerWidgetClass;

	/** Pixel offset after projecting target (widget alignment is center). */
	UPROPERTY(EditAnywhere, Category = "LockOn|Marker")
	FVector2D MarkerScreenOffset = FVector2D::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "LockOn|Debug")
	bool bDrawDebugLockOn = false;

private:
	void BroadcastLockOnChanged(AActor* NewTarget, bool bSuccess);

	UPROPERTY(Transient)
	TObjectPtr<AActor> CurrentTarget = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> ActiveMarkerWidget = nullptr;

	bool bCanSwitchTarget = true;
};
