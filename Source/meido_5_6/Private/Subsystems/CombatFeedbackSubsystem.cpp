#include "Subsystems/CombatFeedbackSubsystem.h"
#include "Camera/CameraShakeBase.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

void UCombatFeedbackSubsystem::ApplyHitStop(AActor* Actor, const float Duration, const float TimeDilation)
{
	Actor->CustomTimeDilation = FMath::Clamp(TimeDilation, 0.001f, 1.f);

	FTimerHandle& Handle = ActiveHitStops.FindOrAdd(Actor);
	GetWorld()->GetTimerManager().SetTimer(
		Handle,
		// the actor will be redirected to EndHitStop as parameter
		FTimerDelegate::CreateUObject(this, &UCombatFeedbackSubsystem::EndHitStop, TWeakObjectPtr<AActor>(Actor)),
		Duration,
		false
	);
}

void UCombatFeedbackSubsystem::PlayCameraShake(const TSubclassOf<UCameraShakeBase> ShakeClass, const float Scale)
{
	if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0))
	{
		PlayerController->ClientStartCameraShake(ShakeClass, Scale);
	}
}

void UCombatFeedbackSubsystem::Deinitialize()
{
	for (TPair<TWeakObjectPtr<AActor>, FTimerHandle>& Pair : ActiveHitStops)
	{
		GetWorld()->GetTimerManager().ClearTimer(Pair.Value);

		if (AActor* Actor = Pair.Key.Get())
		{
			Actor->CustomTimeDilation = 1.f;
		}
	}

	ActiveHitStops.Reset();
	Super::Deinitialize();
}

void UCombatFeedbackSubsystem::EndHitStop(const TWeakObjectPtr<AActor> Actor)
{
	if (AActor* ActorToRestore = Actor.Get())
	{
		ActorToRestore->CustomTimeDilation = 1.f;
	}

	ActiveHitStops.Remove(Actor);
}
