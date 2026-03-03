// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OmuriceBomb.generated.h"

class UStaticMeshComponent;
class UParticleSystem;
class USoundBase;

UCLASS()
class MEIDO_5_6_API AOmuriceBomb : public AActor
{
	GENERATED_BODY()
	
public:	
	AOmuriceBomb();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(VisibleAnywhere, Category="Bomb")
	TObjectPtr<UStaticMeshComponent> BombMesh;

	UPROPERTY(EditAnywhere, Category="Bomb|Fall")
	float InitialHeightOffset = 1800.f;

	UPROPERTY(EditAnywhere, Category="Bomb|Fall", meta=(ClampMin="0.0"))
	float FallSpeed = 1800.f;

	// Trace channel used to detect ground impact while falling.
	// For robust setup, assign a custom channel blocked only by walkable ground.
	UPROPERTY(EditAnywhere, Category="Bomb|Collision")
	TEnumAsByte<ECollisionChannel> GroundTraceChannel = ECC_WorldStatic;

	// Minimum upward normal to treat a blocking hit as "ground-like" impact.
	// Helps ignore side walls/buildings while still impacting on floor surfaces.
	UPROPERTY(EditAnywhere, Category="Bomb|Fall", meta=(ClampMin="0.0", ClampMax="1.0"))
	float GroundImpactMinNormalZ = 0.55f;

	UPROPERTY(EditAnywhere, Category="Bomb|Impact", meta=(ClampMin="0.0"))
	float ImpactDamage = 50.f;

	UPROPERTY(EditAnywhere, Category="Bomb|Impact", meta=(ClampMin="0.0"))
	float ImpactRadius = 450.f;

	UPROPERTY(EditAnywhere, Category="Bomb|Impact|HitStop")
	bool bApplyImpactHitStop = true;

	UPROPERTY(EditAnywhere, Category="Bomb|Impact|HitStop", meta=(ClampMin="0.0", ClampMax="0.2", EditCondition="bApplyImpactHitStop"))
	float ImpactHitStopDuration = 0.05f;

	UPROPERTY(EditAnywhere, Category="Bomb|Impact|HitStop", meta=(ClampMin="0.001", ClampMax="1.0", EditCondition="bApplyImpactHitStop"))
	float ImpactHitStopTimeDilation = 0.01f;

	UPROPERTY(EditAnywhere, Category="Bomb|Impact|HitStop", meta=(EditCondition="bApplyImpactHitStop"))
	bool bApplyImpactHitStopToOwner = true;

	// Time to stay alive after first valid impact.
	UPROPERTY(EditAnywhere, Category="Bomb|Impact", meta=(ClampMin="0.0"))
	float DestroyDelay = 2.0f;

	UPROPERTY(EditAnywhere, Category="Bomb|Impact")
	TObjectPtr<UParticleSystem> ImpactVFX = nullptr;

	UPROPERTY(EditAnywhere, Category="Bomb|Impact")
	TObjectPtr<USoundBase> ImpactSFX = nullptr;

	UPROPERTY(EditAnywhere, Category="Bomb|Debug")
	bool bDrawImpactDebug = false;

	bool bHasImpacted = false;

	void HandleImpact();

};
