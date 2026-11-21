// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MeiDouResults/MeiDouResult.h"
#include "OmuriceBomb.generated.h"

class UProjectileMovementComponent;
class USphereComponent;
class UStaticMeshComponent;

// spawns above the target on InitialHeightOffset and falls
// collision only blocks WorldStatic, so it falls through characters and explodes on the first static surface it hits
UCLASS()
class MEIDO_5_6_API AOmuriceBomb : public AMeiDouResult
{
	GENERATED_BODY()

public:
	AOmuriceBomb();

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void HandleImpact(const FHitResult& ImpactResult);

	UPROPERTY(VisibleAnywhere, Category="Bomb")
	TObjectPtr<USphereComponent> Collision;

	UPROPERTY(VisibleAnywhere, Category="Bomb")
	TObjectPtr<UStaticMeshComponent> BombMesh;
	
	UPROPERTY(VisibleAnywhere, Category="Bomb")
	TObjectPtr<UProjectileMovementComponent> Movement;

	UPROPERTY(EditAnywhere, Category="Bomb|Fall")
	float InitialHeightOffset = 1800.f;

	UPROPERTY(EditAnywhere, Category="Bomb|Impact", meta=(ClampMin="0.0"))
	float ImpactDamage = 50.f;

	UPROPERTY(EditAnywhere, Category="Bomb|Impact", meta=(ClampMin="0.0"))
	float ImpactRadius = 450.f;

	// applied to every actor it damaged, and to the caster if it damaged anyone
	UPROPERTY(EditAnywhere, Category="Bomb|Impact", meta=(ClampMin="0.0", ClampMax="0.2"))
	float ImpactHitStopDuration = 0.05f;

	// time to stay alive after the impact
	UPROPERTY(EditAnywhere, Category="Bomb|Impact", meta=(ClampMin="0.0"))
	float DestroyDelay = 2.0f;
};
