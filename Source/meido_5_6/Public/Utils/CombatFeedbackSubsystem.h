// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CombatFeedbackSubsystem.generated.h"

class AActor;
class AController;
class APlayerController;
class UCameraShakeBase;

UENUM(BlueprintType)
enum class ECombatShakeLevel : uint8
{
	None UMETA(DisplayName = "None"),
	Light UMETA(DisplayName = "Light"),
	Medium UMETA(DisplayName = "Medium"),
	Heavy UMETA(DisplayName = "Heavy")
};

/**
 * World-owned combat presentation (hit-stop + camera shake).
 * Doc: docs/game-mechanics/combat-feedback.md
 */
UCLASS()
class MEIDO_5_6_API UCombatFeedbackSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// --- Hit-stop ---

	UFUNCTION(BlueprintCallable, Category = "Combat|Feedback")
	void ApplyHitStop(
		AActor* Target,
		float Duration = 0.04f,
		float TimeDilation = 0.01f
	);

	UFUNCTION(BlueprintCallable, Category = "Combat|Feedback")
	void ApplyHitStopPair(
		AActor* First,
		AActor* Second,
		float Duration = 0.04f,
		float TimeDilation = 0.01f
	);

	UFUNCTION(BlueprintCallable, Category = "Combat|Feedback")
	void ClearHitStop(AActor* Target);

	UFUNCTION(BlueprintCallable, Category = "Combat|Feedback")
	float ApplyDamageWithHitStop(
		AActor* DamagedActor,
		float BaseDamage,
		AController* EventInstigator,
		AActor* DamageCauser,
		float HitStopDuration = 0.04f,
		float HitStopTimeDilation = 0.01f,
		bool bApplyHitStopToCauser = true
	);

	// --- Shake (CP1.5) ---

	/**
	 * Play a camera shake on the local player controller.
	 * Assign classes on the pawn (or pass explicitly); None / null class = no-op.
	 */
	UFUNCTION(BlueprintCallable, Category = "Combat|Feedback|Shake")
	void PlayCombatShake(
		APlayerController* PlayerController,
		TSubclassOf<UCameraShakeBase> ShakeClass,
		float Scale = 1.f
	);

	/**
	 * Feedback when HP was reduced: optional hit-stop on victim + shake if local player.
	 */
	UFUNCTION(BlueprintCallable, Category = "Combat|Feedback")
	void NotifyDamageReceived(
		AActor* Victim,
		float AppliedDamage,
		TSubclassOf<UCameraShakeBase> ShakeClass,
		float HitStopDuration = 0.03f,
		float HitStopTimeDilation = 0.05f,
		bool bApplyHitStopToVictim = true
	);

	// --- Defaults ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Feedback|HitStop", meta = (ClampMin = "0.001", ClampMax = "1.0"))
	float DefaultHitStopTimeDilation = 0.01f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Feedback|HitStop", meta = (ClampMin = "0.0", ClampMax = "0.2"))
	float LightHitStopDuration = 0.04f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Feedback|HitStop", meta = (ClampMin = "0.0", ClampMax = "0.3"))
	float HeavyHitStopDuration = 0.08f;

	/** Default hit-stop on the victim when NotifyDamageReceived runs. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Feedback|HitStop", meta = (ClampMin = "0.0", ClampMax = "0.2"))
	float DamageReceivedHitStopDuration = 0.03f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Feedback|HitStop", meta = (ClampMin = "0.001", ClampMax = "1.0"))
	float DamageReceivedHitStopTimeDilation = 0.05f;

	UFUNCTION(BlueprintPure, Category = "Combat|Feedback|HitStop")
	float GetLightHitStopDuration() const { return LightHitStopDuration; }

	UFUNCTION(BlueprintPure, Category = "Combat|Feedback|HitStop")
	float GetHeavyHitStopDuration() const { return HeavyHitStopDuration; }

protected:
	virtual void Deinitialize() override;

private:
	TMap<TWeakObjectPtr<AActor>, FTimerHandle> ActiveHitStopTimers;

	void RestoreTimeDilation(AActor* Target);
	void ClearAllHitStops();
};
