#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CombatFeedbackSubsystem.generated.h"

class AActor;
class UCameraShakeBase;

UCLASS()
class MEIDO_5_6_API UCombatFeedbackSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void ApplyHitStop(AActor* Actor, float Duration = 0.04f, float TimeDilation = 0.01f);

	// only applies to local player (index 0) camera
	UFUNCTION(BlueprintCallable)
	void PlayCameraShake(TSubclassOf<UCameraShakeBase> ShakeClass, float Scale = 1.f);

protected:
	virtual void Deinitialize() override;

private:
	void EndHitStop(TWeakObjectPtr<AActor> Actor);

	// actors WITH hitstop
	TMap<TWeakObjectPtr<AActor>, FTimerHandle> ActiveHitStops;
};
