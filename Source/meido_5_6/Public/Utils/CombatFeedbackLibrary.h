#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CombatFeedbackLibrary.generated.h"

UCLASS()
class MEIDO_5_6_API UCombatFeedbackLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Combat|Feedback", meta=(WorldContext="WorldContextObject"))
	static void ApplyLocalHitStop(
		UObject* WorldContextObject,
		AActor* TargetActor,
		float Duration = 0.04f,
		float TimeDilation = 0.01f
	);

	UFUNCTION(BlueprintCallable, Category="Combat|Feedback", meta=(WorldContext="WorldContextObject"))
	static void ApplyLocalHitStopPair(
		UObject* WorldContextObject,
		AActor* FirstActor,
		AActor* SecondActor,
		float Duration = 0.04f,
		float TimeDilation = 0.01f
	);

	UFUNCTION(BlueprintCallable, Category="Combat|Feedback", meta=(WorldContext="WorldContextObject"))
	static void ClearLocalHitStop(UObject* WorldContextObject, AActor* TargetActor);

	UFUNCTION(BlueprintCallable, Category="Combat|Feedback", meta=(WorldContext="WorldContextObject"))
	static float ApplyDamageWithHitStop(
		UObject* WorldContextObject,
		AActor* DamagedActor,
		float BaseDamage,
		AController* EventInstigator,
		AActor* DamageCauser,
		float HitStopDuration = 0.04f,
		float HitStopTimeDilation = 0.01f,
		bool bApplyHitStopToCauser = true
	);
};
