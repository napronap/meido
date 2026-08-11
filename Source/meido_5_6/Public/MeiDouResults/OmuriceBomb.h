// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OmuriceBomb.generated.h"

class UStaticMeshComponent;
class UParticleSystem;
class USoundBase;

/**
 * MMK result (CP3.3): grow → explode → fade. No sky fall / ground tick trace.
 * Doc: docs/game-mechanics/omurice-bomb.md
 *
 * C++ owns timing, scale, radial damage, hit-stop.
 * BP/Content: mesh, VFX/SFX, optional dissolve in OnStartFade.
 */
UCLASS()
class MEIDO_5_6_API AOmuriceBomb : public AActor
{
	GENERATED_BODY()

public:
	AOmuriceBomb();

	/** Force explode if still growing (e.g. external cancel). Safe if already exploded. */
	UFUNCTION(BlueprintCallable, Category = "Omurice")
	void Explode();

	/** BP dissolve finished early — destroy now. Also called when FadeDuration elapses. */
	UFUNCTION(BlueprintCallable, Category = "Omurice")
	void FinishAndDestroy();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	/** Called once after explode; default starts fade timer → destroy. Override in BP for MID dissolve. */
	UFUNCTION(BlueprintNativeEvent, Category = "Omurice")
	void OnStartFade();
	virtual void OnStartFade_Implementation();

	UPROPERTY(VisibleAnywhere, Category = "Omurice")
	TObjectPtr<UStaticMeshComponent> BombMesh;

	// --- Grow ---
	UPROPERTY(EditAnywhere, Category = "Omurice|Grow", meta = (ClampMin = "0.01"))
	float GrowStartScale = 0.15f;

	UPROPERTY(EditAnywhere, Category = "Omurice|Grow", meta = (ClampMin = "0.01"))
	float GrowEndScale = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Omurice|Grow", meta = (ClampMin = "0.05"))
	float GrowDuration = 0.55f;

	// --- Placement ---
	/** One-shot line trace down at spawn to rest on floor (not a continuous fall). */
	UPROPERTY(EditAnywhere, Category = "Omurice|Placement")
	bool bSnapToGroundOnSpawn = true;

	UPROPERTY(EditAnywhere, Category = "Omurice|Placement", meta = (EditCondition = "bSnapToGroundOnSpawn", ClampMin = "0.0"))
	float GroundSnapUpOffset = 100.f;

	UPROPERTY(EditAnywhere, Category = "Omurice|Placement", meta = (EditCondition = "bSnapToGroundOnSpawn", ClampMin = "0.0"))
	float GroundSnapDownDistance = 2500.f;

	UPROPERTY(EditAnywhere, Category = "Omurice|Placement", meta = (EditCondition = "bSnapToGroundOnSpawn"))
	float GroundSnapZOffset = 5.f;

	// --- Explode ---
	UPROPERTY(EditAnywhere, Category = "Omurice|Explode", meta = (ClampMin = "0.0"))
	float ImpactDamage = 50.f;

	UPROPERTY(EditAnywhere, Category = "Omurice|Explode", meta = (ClampMin = "0.0"))
	float ImpactRadius = 450.f;

	UPROPERTY(EditAnywhere, Category = "Omurice|Explode|HitStop")
	bool bApplyImpactHitStop = true;

	UPROPERTY(EditAnywhere, Category = "Omurice|Explode|HitStop", meta = (ClampMin = "0.0", ClampMax = "0.2", EditCondition = "bApplyImpactHitStop"))
	float ImpactHitStopDuration = 0.05f;

	UPROPERTY(EditAnywhere, Category = "Omurice|Explode|HitStop", meta = (ClampMin = "0.001", ClampMax = "1.0", EditCondition = "bApplyImpactHitStop"))
	float ImpactHitStopTimeDilation = 0.01f;

	UPROPERTY(EditAnywhere, Category = "Omurice|Explode|HitStop", meta = (EditCondition = "bApplyImpactHitStop"))
	bool bApplyImpactHitStopToOwner = true;

	UPROPERTY(EditAnywhere, Category = "Omurice|Explode")
	TObjectPtr<UParticleSystem> ImpactVFX = nullptr;

	UPROPERTY(EditAnywhere, Category = "Omurice|Explode")
	TObjectPtr<USoundBase> ImpactSFX = nullptr;

	// --- Fade ---
	/** Default C++ path: wait this long after explode then Destroy. BP dissolve can call FinishAndDestroy sooner. */
	UPROPERTY(EditAnywhere, Category = "Omurice|Fade", meta = (ClampMin = "0.0"))
	float FadeDuration = 0.6f;

	UPROPERTY(EditAnywhere, Category = "Omurice|Debug")
	bool bDrawImpactDebug = false;

private:
	enum class EPhase : uint8
	{
		Growing,
		Fading,
		Done
	};

	EPhase Phase = EPhase::Growing;
	float GrowElapsed = 0.f;
	bool bHasExploded = false;

	void SnapToGround();
	void ApplyGrowScale(float Alpha);
	void ApplyExplosionDamageAndFeedback();
};
