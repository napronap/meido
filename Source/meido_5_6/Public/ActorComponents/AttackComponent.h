// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AttackComponent.generated.h"

class ACharacter;

UENUM(BlueprintType)
enum class EHitStopType : uint8
{
	Light UMETA(DisplayName = "Light"),
	Heavy UMETA(DisplayName = "Heavy")
};

// As it is right now, this AC only works for the maids attacks as it relies on hitboxes mounted on sockets
// and the hit window anim notify also relies on that.
// could be expanded to take an actor to where to attach the box to I guess
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MEIDO_5_6_API UAttackComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAttackComponent();

	// starts tracing from these sockets every frame until CloseHitWindow
	void OpenHitWindow(const TArray<FName>& Sockets, EHitStopType HitStopType);
	void CloseHitWindow();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;

private:
	void PerformHitTrace();
	bool ShouldIgnoreHitActor(const AActor* HitActor) const;

	UPROPERTY()
	TObjectPtr<ACharacter> OwnerCharacter;

	UPROPERTY(EditAnywhere, Category="Attack")
	float Damage = 20.f;

	// half size of the box swept along each socket's path
	// box is oriented to the socket
	// default is a size that makes sense for the maid's hand
	UPROPERTY(EditAnywhere, Category="Attack")
	FVector BoxExtent = FVector(7.f, 10.f, 10.f);

	UPROPERTY(EditAnywhere, Category="Attack|HitStop", meta=(ClampMin="0.0", ClampMax="0.2"))
	float LightHitStopDuration = 0.04f;

	UPROPERTY(EditAnywhere, Category="Attack|HitStop", meta=(ClampMin="0.0", ClampMax="0.3"))
	float HeavyHitStopDuration = 0.08f;

	TArray<FName> CurrentHitSockets;
	// where each socket was last frame (same index as CurrentHitSockets)
	TArray<FVector> PreviousSocketLocations;
	
	EHitStopType CurrentHitStopType = EHitStopType::Light;
	TSet<TWeakObjectPtr<AActor>> HitActorsThisWindow;
};
