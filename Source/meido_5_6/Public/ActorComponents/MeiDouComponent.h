// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MeiDouComboData.h"
#include "Components/ActorComponent.h"
#include "Types/MeiDouTypes.h"
#include "MeiDouComponent.generated.h"

USTRUCT(BlueprintType)
struct FMeiDouResolvedCombo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FName ComboId;

	UPROPERTY(BlueprintReadOnly)
	TArray<EMeiDouInput> Inputs;
};

USTRUCT(BlueprintType)
struct FMeiDouPoseAnimationRequest
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	EMeiDouInput Input = EMeiDouInput::None;

	UPROPERTY(BlueprintReadOnly)
	bool bUseMirroredMontage = false;
};

// APPARENTLY I need to set it as dynamic so that later on I can do stuff on BP
// I'm not sure if I'll have to, but let's keep it like this for now 
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnMeiDouComboResolved,
	const FMeiDouResolvedCombo&, Result
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnMeiDouPoseAnimationRequested,
	const FMeiDouPoseAnimationRequest&, Request
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnMeiDouStateChanged,
	EMeiDouState, NewState
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnMeiDouControlLockChanged,
	bool, bIsLocked
);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MEIDO_5_6_API UMeiDouComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UMeiDouComponent();

	// register the pose input
	bool RegisterInput(EMeiDouInput Input);

	// Called by animation notify state to indicate current MeiDou action lifecycle.
	void OnMeiDouActionWindowBegin();
	void OnMeiDouActionWindowEnd();

	// Called by animation owner if a requested montage could not be played.
	void OnRequestedAnimationFailed();

	// Handles generic animation events emitted by MeiDou notifies.
	void HandleAnimEvent(EMeiDouAnimEvent EventKey);

	// Returns the currently resolved combo definition while its result montage is active.
	const FMeiDouComboDefinition* GetActiveComboDefinition() const;

	EMeiDouState GetMeiDouState() const { return MeiDouState; }
	bool IsMeiDouActive() const { return MeiDouState != EMeiDouState::EMDS_Idle; }

	// reset the combo upon completion, damage, etc
	void ResetCombo();

	// Delegate that broadcasts the output of the combo
	// again, not sure if this will eventually be used in blueprints anyways
	UPROPERTY(BlueprintAssignable)
	FOnMeiDouComboResolved OnComboResolved;

	UPROPERTY(BlueprintAssignable)
	FOnMeiDouPoseAnimationRequested OnPoseAnimationRequested;

	UPROPERTY(BlueprintAssignable)
	FOnMeiDouStateChanged OnMeiDouStateChanged;

	UPROPERTY(BlueprintAssignable)
	FOnMeiDouControlLockChanged OnMeiDouControlLockChanged;

	UPROPERTY(EditDefaultsOnly)
	UMeiDouComboData* ComboData;
	
	const TArray<EMeiDouInput>& GetInputBuffer() const;
	
	// Will return the amount of times the input argument is inside the current buffer
	int32 GetInputCount(EMeiDouInput Input) const;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	TArray<EMeiDouInput> ComboInputBuffer;

	UPROPERTY()
	TArray<EMeiDouInput> QueuedPoseInputs;
	
	// no plans of making this have more than 3 inputs
	int32 MaxInputs = 3;

	UPROPERTY(EditDefaultsOnly, Category="MeiDou")
	float ComboInputTimeoutSeconds = 2.f;

	FTimerHandle ComboInputTimeoutHandle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="MeiDou", meta=(AllowPrivateAccess="true"))
	EMeiDouState MeiDouState = EMeiDouState::EMDS_Idle;

	UPROPERTY()
	FMeiDouComboDefinition ActiveComboDefinition;
	bool bHasActiveComboDefinition = false;

	UPROPERTY()
	FMeiDouComboDefinition PendingResolvedComboDefinition;
	bool bHasPendingResolvedCombo = false;

	UPROPERTY()
	FMeiDouResolvedCombo PendingResolvedComboResult;

	TWeakObjectPtr<AActor> SpawnedResultActor;
	bool bDestroySpawnedResultActorOnActionEnd = false;

	void SetMeiDouState(EMeiDouState NewState);
	bool HasActiveComboDefinition() const;
	bool HasPendingResolvedCombo() const;
	void ClearActiveComboDefinition();
	void ClearPendingResolvedCombo();
	void CleanupSpawnedResultActor();
	void RestartComboInputTimeout();
	void ClearComboInputTimeout();
	void OnComboInputTimeout();
	void TryConsumeNextQueuedAction();
	bool RegisterInputToComboBuffer(EMeiDouInput Input);
	bool TryResolve();
	void ExecuteResolvedCombo(const FMeiDouComboDefinition& Definition);
	void ExecuteSpawnResult(const FMeiDouSpawnConfig& SpawnConfig);
	FVector GetSpawnLocationForConfig(const FMeiDouSpawnConfig& SpawnConfig, AActor*& OutTargetActor);
	AActor* GetCurrentLockOnTarget();
};
