#include "IA/CombatDirectorSubsystem.h"

bool UCombatDirectorSubsystem::RequestAttackSlot(AActor* Requester)
{
	if (!Requester)
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
		if (Entry.IsValid())
		{
			++ValidCount;
		}
	}

	return ValidCount;
}

void UCombatDirectorSubsystem::CleanupInvalidAttackers()
{
	ActiveAttackers.RemoveAll(
		[](const TWeakObjectPtr<AActor>& Entry)
		{
			return !Entry.IsValid();
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

