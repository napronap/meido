#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CombatDirectorSubsystem.generated.h"

class AActor;

UCLASS()
class MEIDO_5_6_API UCombatDirectorSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
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

	void CleanupInvalidAttackers();
	bool ContainsRequester(AActor* Requester) const;
};
