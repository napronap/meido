// Fill out your copyright notice in the Description page of Project Settings.

#include "Utils/CombatFeedbackSubsystem.h"
#include "Camera/CameraShakeBase.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

void UCombatFeedbackSubsystem::Deinitialize()
{
	ClearAllHitStops();
	Super::Deinitialize();
}

void UCombatFeedbackSubsystem::ApplyHitStop(
	AActor* Target,
	const float Duration,
	const float TimeDilation
)
{
	if (!Target || Duration <= 0.f)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	Target->CustomTimeDilation = FMath::Clamp(TimeDilation, 0.001f, 1.0f);

	FTimerHandle& TimerHandle = ActiveHitStopTimers.FindOrAdd(Target);
	World->GetTimerManager().ClearTimer(TimerHandle);

	const TWeakObjectPtr<AActor> WeakTarget = Target;
	const TWeakObjectPtr<UCombatFeedbackSubsystem> WeakThis = this;

	FTimerDelegate RestoreDelegate;
	RestoreDelegate.BindLambda([WeakThis, WeakTarget]()
	{
		if (UCombatFeedbackSubsystem* Self = WeakThis.Get())
		{
			if (AActor* ActorToRestore = WeakTarget.Get())
			{
				Self->RestoreTimeDilation(ActorToRestore);
			}
			Self->ActiveHitStopTimers.Remove(WeakTarget);
		}
	});

	World->GetTimerManager().SetTimer(TimerHandle, RestoreDelegate, Duration, false);
}

void UCombatFeedbackSubsystem::ApplyHitStopPair(
	AActor* First,
	AActor* Second,
	const float Duration,
	const float TimeDilation
)
{
	ApplyHitStop(First, Duration, TimeDilation);
	ApplyHitStop(Second, Duration, TimeDilation);
}

void UCombatFeedbackSubsystem::ClearHitStop(AActor* Target)
{
	if (!Target)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (World)
	{
		if (FTimerHandle* TimerHandle = ActiveHitStopTimers.Find(Target))
		{
			World->GetTimerManager().ClearTimer(*TimerHandle);
			ActiveHitStopTimers.Remove(Target);
		}
	}

	RestoreTimeDilation(Target);
}

float UCombatFeedbackSubsystem::ApplyDamageWithHitStop(
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

	ApplyHitStop(DamagedActor, HitStopDuration, HitStopTimeDilation);

	if (bApplyHitStopToCauser && DamageCauser && DamageCauser != DamagedActor)
	{
		ApplyHitStop(DamageCauser, HitStopDuration, HitStopTimeDilation);
	}

	return AppliedDamage;
}

void UCombatFeedbackSubsystem::PlayCombatShake(
	APlayerController* PlayerController,
	const TSubclassOf<UCameraShakeBase> ShakeClass,
	const float Scale
)
{
	if (!PlayerController || !ShakeClass || Scale <= 0.f)
	{
		return;
	}

	if (!PlayerController->IsLocalController())
	{
		return;
	}

	PlayerController->ClientStartCameraShake(ShakeClass, Scale);
}

void UCombatFeedbackSubsystem::NotifyDamageReceived(
	AActor* Victim,
	const float AppliedDamage,
	const TSubclassOf<UCameraShakeBase> ShakeClass,
	const float HitStopDuration,
	const float HitStopTimeDilation,
	const bool bApplyHitStopToVictim
)
{
	if (!Victim || AppliedDamage <= 0.f)
	{
		return;
	}

	if (bApplyHitStopToVictim)
	{
		const float Duration = HitStopDuration > 0.f ? HitStopDuration : DamageReceivedHitStopDuration;
		const float Dilation = HitStopTimeDilation > 0.f ? HitStopTimeDilation : DamageReceivedHitStopTimeDilation;
		ApplyHitStop(Victim, Duration, Dilation);
	}

	const APawn* VictimPawn = Cast<APawn>(Victim);
	if (!VictimPawn)
	{
		return;
	}

	APlayerController* PC = Cast<APlayerController>(VictimPawn->GetController());
	if (!PC)
	{
		return;
	}

	PlayCombatShake(PC, ShakeClass, 1.f);
}

void UCombatFeedbackSubsystem::RestoreTimeDilation(AActor* Target)
{
	if (Target)
	{
		Target->CustomTimeDilation = 1.0f;
	}
}

void UCombatFeedbackSubsystem::ClearAllHitStops()
{
	UWorld* World = GetWorld();
	if (World)
	{
		for (auto& Pair : ActiveHitStopTimers)
		{
			World->GetTimerManager().ClearTimer(Pair.Value);
			if (AActor* Target = Pair.Key.Get())
			{
				RestoreTimeDilation(Target);
			}
		}
	}

	ActiveHitStopTimers.Reset();
}
