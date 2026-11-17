// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MeiDouComboData.h"
#include "Components/ActorComponent.h"
#include "Types/MeiDouTypes.h"
#include "MeiDouComponent.generated.h"

class AMaidCharacter;
class UMeiDouPoseDataAsset;
class UAnimMontage;
class USoundBase;

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

// Multicast: multi-listener (UI / audio). Presentation play is cast to owner (CP3.1).
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnMeiDouComboResolved,
	const FMeiDouResolvedCombo&, Result
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnMeiDouComboFailed,
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

/** Why the combo buffer changed (CP3.5 UI). */
UENUM(BlueprintType)
enum class EMeiDouBufferChangeReason : uint8
{
	PoseAdded UMETA(DisplayName = "PoseAdded"),
	ClearedSuccess UMETA(DisplayName = "ClearedSuccess"),
	ClearedFail UMETA(DisplayName = "ClearedFail"),
	ClearedTimeout UMETA(DisplayName = "ClearedTimeout"),
	ClearedOther UMETA(DisplayName = "ClearedOther"),
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnMeiDouBufferChanged,
	const TArray<EMeiDouInput>&, Buffer,
	EMeiDouBufferChangeReason, Reason
);

/**
 * MeiDou gimmick orchestrator (CP3.1–3.5).
 *
 * FSM: Idle → Posing → (queue more poses) → Result | Fail → Idle
 * Catalog: Recipes + legacy Combos. Presentation: cast to AMaidCharacter.
 * Preload: soft result classes on BeginPlay.
 * CP3.4: inter-pose timeout only while Idle waiting; mirror full-buffer count.
 * CP3.5: OnMeiDouBufferChanged for HUD queue (bubble is player-only WidgetComponent).
 * CP3.6: pose SFX on UMeiDouPoseDataAsset; success/fail on component; result SFX on result actors.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MEIDO_5_6_API UMeiDouComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMeiDouComponent();

	bool RegisterInput(EMeiDouInput Input);

	void OnMeiDouActionWindowBegin();
	void OnMeiDouActionWindowEnd();
	void OnRequestedAnimationFailed();
	void HandleAnimEvent(EMeiDouAnimEvent EventKey);

	const FMeiDouComboDefinition* GetActiveComboDefinition() const;

	EMeiDouState GetMeiDouState() const { return MeiDouState; }
	bool IsMeiDouActive() const { return MeiDouState != EMeiDouState::EMDS_Idle; }

	void ResetCombo();
	/** Full run reset: queued poses, result actors and timeout must not survive retry. */
	void ResetForFlowRestart();

	/** Clear buffer and broadcast (default ClearedOther). */
	void ResetComboAndNotify(EMeiDouBufferChangeReason Reason);

	/**
	 * CP3.2: Load soft spawn classes from ComboData (Recipes + legacy Combos).
	 * Safe to call more than once. Default: BeginPlay when bPreloadResultsOnBeginPlay.
	 * Not owned by GameMode — any map with this component warms results.
	 */
	UFUNCTION(BlueprintCallable, Category = "MeiDou|Preload")
	void PreloadResults();

	UFUNCTION(BlueprintPure, Category = "MeiDou|Preload")
	bool HasPreloadedResults() const { return bResultsPreloaded; }

	UPROPERTY(BlueprintAssignable, Category = "MeiDou|Events")
	FOnMeiDouComboResolved OnComboResolved;

	UPROPERTY(BlueprintAssignable, Category = "MeiDou|Events")
	FOnMeiDouComboFailed OnComboFailed;

	/** Still fired for UI; pose play is also driven via owner cast. */
	UPROPERTY(BlueprintAssignable, Category = "MeiDou|Events")
	FOnMeiDouPoseAnimationRequested OnPoseAnimationRequested;

	UPROPERTY(BlueprintAssignable, Category = "MeiDou|Events")
	FOnMeiDouStateChanged OnMeiDouStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "MeiDou|Events")
	FOnMeiDouControlLockChanged OnMeiDouControlLockChanged;

	/** Buffer snapshot after pose add / clear (success, fail, timeout). HUD queue listens here. */
	UPROPERTY(BlueprintAssignable, Category = "MeiDou|Events")
	FOnMeiDouBufferChanged OnMeiDouBufferChanged;

	UPROPERTY(EditDefaultsOnly, Category = "MeiDou|Data")
	TObjectPtr<UMeiDouComboData> ComboData;

	/**
	 * Pose montages for this caster (player/boss). Prefer config here over character base.
	 * If empty, presentation falls back to AMaidCharacter::MeiDouPoseDataMap.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "MeiDou|Presentation")
	TMap<EMeiDouInput, TObjectPtr<UMeiDouPoseDataAsset>> PoseDataMap;

	/**
	 * Fail / berrinche montage. Prefer here; falls back to character MeiDouFailMontage.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "MeiDou|Presentation")
	TObjectPtr<UAnimMontage> FailMontage;

	/** Cast-layer success (not result boom/beam). Null = silence. */
	UPROPERTY(EditDefaultsOnly, Category = "MeiDou|Audio")
	TObjectPtr<USoundBase> SuccessSfx;

	/** Cast-layer fail / berrinche. Null = silence. */
	UPROPERTY(EditDefaultsOnly, Category = "MeiDou|Audio")
	TObjectPtr<USoundBase> FailSfx;

	UPROPERTY(EditDefaultsOnly, Category = "MeiDou|Debug")
	bool bDrawDebugMeiDou = false;

	/** If true, PreloadResults runs in BeginPlay (demo + playground). */
	UPROPERTY(EditDefaultsOnly, Category = "MeiDou|Preload")
	bool bPreloadResultsOnBeginPlay = true;

	const TArray<EMeiDouInput>& GetInputBuffer() const;

	/** How many times Input appears in the current combo buffer (full string). */
	int32 GetInputCount(EMeiDouInput Input) const;

	/**
	 * CP3.4 mirror: true for 2nd, 4th… occurrence of Input in the buffer
	 * (count already in buffer before this press is odd → mirror).
	 */
	UFUNCTION(BlueprintPure, Category = "MeiDou")
	bool ShouldMirrorPose(EMeiDouInput Input) const;

	const UMeiDouPoseDataAsset* FindPoseData(EMeiDouInput Input) const;
	UAnimMontage* GetFailMontage() const;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	TArray<EMeiDouInput> ComboInputBuffer;

	/** Player inputs waiting to become poses (max MaxInputs). */
	UPROPERTY()
	TArray<EMeiDouInput> QueuedPoseInputs;

	int32 MaxInputs = 3;

	/**
	 * CP3.4: seconds to wait for the *next* pose after the previous pose window ends
	 * (Idle + partial buffer). Not ticked during Posing/Result. 0 = disabled.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "MeiDou|Timeout", meta = (ClampMin = "0.0"))
	float ComboInputTimeoutSeconds = 2.f;

	/**
	 * If true (default), never keep an inter-pose timer running while Posing/Result
	 * (policy: pause while busy). Timer only arms when Idle and waiting for input.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "MeiDou|Timeout")
	bool bTimeoutOnlyWhileIdleWaiting = true;

	FTimerHandle ComboInputTimeoutHandle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MeiDou", meta = (AllowPrivateAccess = "true"))
	EMeiDouState MeiDouState = EMeiDouState::EMDS_Idle;

	UPROPERTY()
	FMeiDouComboDefinition ActiveComboDefinition;
	bool bHasActiveComboDefinition = false;

	UPROPERTY()
	FMeiDouComboDefinition PendingResultDefinition;
	bool bHasPendingResult = false;

	UPROPERTY()
	FMeiDouResolvedCombo PendingResultPayload;

	UPROPERTY()
	FMeiDouResolvedCombo PendingFailPayload;
	bool bHasPendingFail = false;

	TWeakObjectPtr<AActor> SpawnedResultActor;
	TArray<TWeakObjectPtr<AActor>> RunSpawnedResultActors;
	bool bDestroySpawnedResultActorOnActionEnd = false;

	bool bResultsPreloaded = false;

	void SetMeiDouState(EMeiDouState NewState);
	void ClearActiveComboDefinition();
	void ClearPendingResult();
	void ClearPendingFail();
	void CleanupSpawnedResultActor();
	/** Arm timer only if Idle, partial buffer, no queued poses / pending result-fail. */
	void ArmInterPoseTimeoutIfWaiting();
	void ClearComboInputTimeout();
	void OnComboInputTimeout();

	/** Single advance entry: only runs while Idle. Priority: pose queue → result → fail → arm timeout. */
	void AdvanceFromIdle();

	bool AppendToComboBuffer(EMeiDouInput Input);
	bool TryResolveBuffer();
	void QueueSuccess(const FMeiDouComboDefinition& Definition);
	void QueueFail();
	void ExecuteSpawnResult(const FMeiDouSpawnConfig& SpawnConfig);
	FVector GetSpawnLocationForConfig(const FMeiDouSpawnConfig& SpawnConfig, AActor*& OutTargetActor);
	AActor* GetCurrentLockOnTarget();

	AMaidCharacter* GetMaidOwner() const;
	void DebugMessage(const FString& Message, const FColor& Color, float Time = 2.f) const;

	void PresentPose(const FMeiDouPoseAnimationRequest& Request);
	void PresentResult(const FMeiDouResolvedCombo& Result);
	void PresentFail(const FMeiDouResolvedCombo& Result);

	void NotifyBufferChanged(EMeiDouBufferChangeReason Reason);

	void PlayMeiDouSfx(USoundBase* Sound) const;
	void PlayPoseSfx(EMeiDouInput Input, bool bMirrored) const;
};
