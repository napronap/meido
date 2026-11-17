#include "IA/CombatDirectorSubsystem.h"
#include "Characters/MaidCharacter.h"

bool UCombatDirectorSubsystem::IsEligibleActor(const AActor* InCandidate) const
{
	if (!IsValid(InCandidate) || InCandidate->IsActorBeingDestroyed() || InCandidate->GetWorld() != GetWorld())
	{
		return false;
	}
	const AMaidCharacter* CandidateMaid = Cast<AMaidCharacter>(InCandidate);
	return !CandidateMaid || !CandidateMaid->IsDead();
}

void UCombatDirectorSubsystem::Deinitialize()
{
	ActiveAttackers.Reset();
	HoldAngles.Reset();
	NextHoldAngleIndex = 0;
	Super::Deinitialize();
}

bool UCombatDirectorSubsystem::HasAttackSlot(AActor* InRequester) const
{
	return IsEligibleActor(InRequester) && ContainsRequester(InRequester);
}

void UCombatDirectorSubsystem::SetMaxSimultaneousAttackers(const int32 InMaxAttackers)
{
	MaxSimultaneousAttackers = FMath::Max(1, InMaxAttackers);
	CleanupInvalidAttackers();
}

float UCombatDirectorSubsystem::RegisterHoldAngle(AActor* InRequester)
{
	if (!IsEligibleActor(InRequester))
	{
		return 0.f;
	}
	for (auto AngleIt = HoldAngles.CreateIterator(); AngleIt; ++AngleIt)
	{
		if (!IsEligibleActor(AngleIt.Key().Get()))
		{
			AngleIt.RemoveCurrent();
		}
	}
	const TWeakObjectPtr<AActor> RequesterKey(InRequester);
	if (const float* ExistingAngle = HoldAngles.Find(RequesterKey))
	{
		return *ExistingAngle;
	}
	const float AssignedAngle = FMath::Fmod(static_cast<float>(NextHoldAngleIndex++) * 137.508f, 360.f);
	HoldAngles.Add(RequesterKey, AssignedAngle);
	return AssignedAngle;
}

void UCombatDirectorSubsystem::UnregisterHoldAngle(AActor* InRequester)
{
	HoldAngles.Remove(TWeakObjectPtr<AActor>(InRequester));
}

bool UCombatDirectorSubsystem::RequestAttackSlot(AActor* Requester)
{
	if (!IsEligibleActor(Requester))
	{
		return false;
	}

	CleanupInvalidAttackers();

	if (ContainsRequester(Requester))
	{
		return true;
	}

	if (ActiveAttackers.Num() >= MaxSimultaneousAttackers)
	{
		return false;
	}

	ActiveAttackers.Add(Requester);
	return true;
}

void UCombatDirectorSubsystem::ReleaseAttackSlot(AActor* Requester)
{
	if (!Requester)
	{
		return;
	}

	CleanupInvalidAttackers();
	ActiveAttackers.RemoveAll(
		[Requester](const TWeakObjectPtr<AActor>& Entry)
		{
			return !Entry.IsValid() || Entry.Get() == Requester;
		}
	);
}

int32 UCombatDirectorSubsystem::GetActiveAttackersCount() const
{
	int32 ValidCount = 0;
	for (const TWeakObjectPtr<AActor>& Entry : ActiveAttackers)
	{
		if (IsEligibleActor(Entry.Get()))
		{
			++ValidCount;
		}
	}

	return ValidCount;
}

void UCombatDirectorSubsystem::CleanupInvalidAttackers()
{
	ActiveAttackers.RemoveAll(
		[this](const TWeakObjectPtr<AActor>& Entry)
		{
			return !IsEligibleActor(Entry.Get());
		}
	);
}

bool UCombatDirectorSubsystem::ContainsRequester(AActor* Requester) const
{
	for (const TWeakObjectPtr<AActor>& Entry : ActiveAttackers)
	{
		if (Entry.IsValid() && Entry.Get() == Requester)
		{
			return true;
		}
	}

	return false;
}
