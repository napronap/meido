// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AttackComponent.generated.h"

UENUM(BlueprintType)
enum class EHitStopType : uint8
{
	Light UMETA(DisplayName = "Light"),
	Heavy UMETA(DisplayName = "Heavy")
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MEIDO_5_6_API UAttackComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAttackComponent();

	UFUNCTION(BlueprintCallable)
	void StartAttack(EHitStopType InHitStopType = EHitStopType::Light);

	UFUNCTION(BlueprintCallable)
	void OpenHitWindow(FName InSocket);

	UFUNCTION(BlueprintCallable)
	void OpenHitWindowSockets(const TArray<FName>& InSockets);

	UFUNCTION(BlueprintCallable)
	void CloseHitWindow();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void PerformHitTrace();
	void ApplyLocalHitStop(AActor* TargetActor, EHitStopType HitStopType);
	void RestoreLocalTimeDilation(AActor* TargetActor);
	float GetHitStopDuration(EHitStopType HitStopType) const;

	UPROPERTY()
	ACharacter* OwnerCharacter;

	UPROPERTY(EditAnywhere, Category="Attack")
	float Damage = 10.f;

	UPROPERTY(EditAnywhere, Category="Attack")
	FVector BoxExtent = FVector(20.f, 20.f, 20.f);
	
	UPROPERTY(EditAnywhere, Category="Attack")
	float TraceDistance = 60.f;

	UPROPERTY(EditAnywhere, Category="Attack")
	FName HitSocketName = "Attack_Hand_R";

	UPROPERTY(EditAnywhere, Category="Attack|HitStop", meta=(ClampMin="0.001", ClampMax="1.0"))
	float HitStopTimeDilation = 0.01f;

	UPROPERTY(EditAnywhere, Category="Attack|HitStop", meta=(ClampMin="0.0", ClampMax="0.2"))
	float LightHitStopDuration = 0.04f;

	UPROPERTY(EditAnywhere, Category="Attack|HitStop", meta=(ClampMin="0.0", ClampMax="0.3"))
	float HeavyHitStopDuration = 0.08f;

	bool bIsAttacking = false;
	bool bHitWindowOpen = false;
	EHitStopType CurrentHitStopType = EHitStopType::Light;

	TSet<AActor*> HitActorsThisAttack;
	TMap<TWeakObjectPtr<AActor>, FTimerHandle> ActiveHitStopTimers;
	
	TArray<FName> CurrentHitSockets;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
