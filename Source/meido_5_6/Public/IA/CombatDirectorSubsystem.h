#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CombatDirectorSubsystem.generated.h"

class AActor;

UCLASS()
class MEIDO_5_6_API UCombatDirectorSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintPure, Category = "CombatDirector")
	bool HasAttackSlot(AActor* InRequester) const;

	/** Normally configured before combat starts by the GameMode. Existing tokens drain naturally. */
	UFUNCTION(BlueprintCallable, Category = "CombatDirector")
	void SetMaxSimultaneousAttackers(int32 InMaxAttackers);

	float RegisterHoldAngle(AActor* InRequester);
	void UnregisterHoldAngle(AActor* InRequester);

	UFUNCTION(BlueprintCallable, Category="CombatDirector")
	bool RequestAttackSlot(AActor* Requester);

	UFUNCTION(BlueprintCallable, Category="CombatDirector")
	void ReleaseAttackSlot(AActor* Requester);

	UFUNCTION(BlueprintPure, Category="CombatDirector")
	int32 GetActiveAttackersCount() const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CombatDirector", meta=(ClampMin="1", UIMin="1"))
	int32 MaxSimultaneousAttackers = 2;

private:
	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<AActor>> ActiveAttackers;
	TMap<TWeakObjectPtr<AActor>, float> HoldAngles;
	int32 NextHoldAngleIndex = 0;
	bool IsEligibleActor(const AActor* InCandidate) const;

	void CleanupInvalidAttackers();
	bool ContainsRequester(AActor* Requester) const;
};
