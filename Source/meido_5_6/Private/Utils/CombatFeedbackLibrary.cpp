#include "Utils/CombatFeedbackLibrary.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

namespace
{
	TMap<TWeakObjectPtr<AActor>, FTimerHandle> GActiveHitStopTimers;
}

void UCombatFeedbackLibrary::ApplyLocalHitStop(
	UObject* WorldContextObject,
	AActor* TargetActor,
	const float Duration,
	const float TimeDilation
)
{
	if (!TargetActor || Duration <= 0.f)
	{
		return;
	}

	UWorld* World = GEngine
		? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull)
		: nullptr;

	if (!World)
	{
		return;
	}

	TargetActor->CustomTimeDilation = FMath::Clamp(TimeDilation, 0.001f, 1.0f);

	FTimerHandle& TimerHandle = GActiveHitStopTimers.FindOrAdd(TargetActor);
	World->GetTimerManager().ClearTimer(TimerHandle);

	const TWeakObjectPtr<AActor> WeakTarget = TargetActor;
	FTimerDelegate RestoreDelegate;
	RestoreDelegate.BindLambda([WeakTarget]()
	{
		if (AActor* ActorToRestore = WeakTarget.Get())
		{
			ActorToRestore->CustomTimeDilation = 1.0f;
		}

		GActiveHitStopTimers.Remove(WeakTarget);
	});

	World->GetTimerManager().SetTimer(TimerHandle, RestoreDelegate, Duration, false);
}

void UCombatFeedbackLibrary::ApplyLocalHitStopPair(
	UObject* WorldContextObject,
	AActor* FirstActor,
	AActor* SecondActor,
	const float Duration,
	const float TimeDilation
)
{
	ApplyLocalHitStop(WorldContextObject, FirstActor, Duration, TimeDilation);
	ApplyLocalHitStop(WorldContextObject, SecondActor, Duration, TimeDilation);
}

void UCombatFeedbackLibrary::ClearLocalHitStop(UObject* WorldContextObject, AActor* TargetActor)
{
	if (!TargetActor)
	{
		return;
	}

	UWorld* World = GEngine
		? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull)
		: nullptr;

	if (!World)
	{
		return;
	}

	if (FTimerHandle* TimerHandle = GActiveHitStopTimers.Find(TargetActor))
	{
		World->GetTimerManager().ClearTimer(*TimerHandle);
		GActiveHitStopTimers.Remove(TargetActor);
	}

	TargetActor->CustomTimeDilation = 1.0f;
}

float UCombatFeedbackLibrary::ApplyDamageWithHitStop(
	UObject* WorldContextObject,
	AActor* DamagedActor,
	const float BaseDamage,
	AController* EventInstigator,
	AActor* DamageCauser,
	const float HitStopDuration,
	const float HitStopTimeDilation,
	const bool bApplyHitStopToCauser
)
{
	if (!DamagedActor || BaseDamage <= 0.f)
	{
		return 0.f;
	}

	const float AppliedDamage = UGameplayStatics::ApplyDamage(
		DamagedActor,
		BaseDamage,
		EventInstigator,
		DamageCauser,
		nullptr
	);

	if (AppliedDamage <= 0.f)
	{
		return 0.f;
	}

	ApplyLocalHitStop(WorldContextObject, DamagedActor, HitStopDuration, HitStopTimeDilation);

	if (bApplyHitStopToCauser && DamageCauser && DamageCauser != DamagedActor)
	{
		ApplyLocalHitStop(WorldContextObject, DamageCauser, HitStopDuration, HitStopTimeDilation);
	}

	return AppliedDamage;
}
