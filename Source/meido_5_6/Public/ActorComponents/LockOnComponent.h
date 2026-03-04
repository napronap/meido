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

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MEIDO_5_6_API ULockOnComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	ULockOnComponent();

	UFUNCTION(BlueprintCallable)
	bool TryLockOn();

	UFUNCTION(BlueprintCallable)
	void ClearLockOn();

	UFUNCTION(BlueprintCallable)
	bool IsLockedOn();

	UFUNCTION(BlueprintCallable)
	AActor* GetCurrentTarget();

	UFUNCTION(BlueprintCallable)
	void HandleSwitchInput(float AxisValue);

	UFUNCTION(BlueprintCallable)
	void HandleSwitchReleased();
	void SwitchTarget(int32 Direction);

	UPROPERTY(BlueprintAssignable)
	FOnLockOnChanged OnLockOnChanged;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	void FindTargets(TArray<AActor*>& OutTargets) const;

	AActor* SelectBestTarget(const TArray<AActor*>& Targets) const;
	bool IsTargetValidForLockOn(const AActor* Target) const;
	bool RefreshCurrentTarget();

	UPROPERTY()
	ACharacter* OwnerCharacter;

	UPROPERTY(EditAnywhere)
	float SearchRadius = 1200.f;

	UPROPERTY(EditAnywhere)
	TArray<TEnumAsByte<EObjectTypeQuery>> TargetObjectType;

	UPROPERTY()
	AActor* CurrentTarget = nullptr;

	bool bCanSwitchTarget = true;

	float SwitchThreshold = 0.7f;
	float NeutralThreshold = 0.4f;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
};
