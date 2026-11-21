// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DataAssets/MeiDouComboData.h"
#include "DataAssets/MeiDouPoseSet.h"
#include "Components/ActorComponent.h"
#include "Types/MeiDouTypes.h"
#include "MeiDouComponent.generated.h"

class ACharacter;
class AMeiDouResult;
class UAnimMontage;
class UAnimSequenceBase;
class UAttackComponent;
class ULockOnComponent;

// MeiDou: three poses in a row resolve into a combo (ComboData) or a fail
//
//  input -> pose montage -> ActionEnd notify: next pose (if queued) or resolve on the 3rd (combo result)
//
//  resolve -> result montage (Spawn / TraceStart / TraceEnd events) -> ActionEnd: back to idle
//          -> no match: fail montage ("stunned" until it blends out)
//
// Flow is the same as attacks: input -> AMaidCharacter (gates) -> RegisterInput (internal checks only) -> AC
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MEIDO_5_6_API UMeiDouComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMeiDouComponent();

	// plays the pose, or queues it while another pose plays (ignored once the 3rd pose or a result is going)
	void RegisterInput(EMeiDouInput Input);

	// called by the MeiDou event notify with the montage that fired it
	void HandleAnimEvent(EMeiDouAnimEvent Event, const UAnimSequenceBase* Animation);

	// posing or playing a result
	bool IsActive() const { return State == EMeiDouState::EMDS_Posing || State == EMeiDouState::EMDS_Result; }

	// fail montage playing
	bool IsFailed() const { return State == EMeiDouState::EMDS_Failed; }

protected:
	virtual void BeginPlay() override;

private:
	void PlayPose(EMeiDouInput Input);
	void Resolve();
	void EndAction();
	void PlayMontage(UAnimMontage* Montage, float PlayRate, FName Section = NAME_None);
	void HandleMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted, int32 InstanceId);
	void ResetCombo();
	void SetMirrorFlag(bool bShouldMirror) const;

	void ExecuteSpawnResult(const FMeiDouSpawnConfig& SpawnConfig);

	UPROPERTY(EditDefaultsOnly, Category="MeiDou")
	TObjectPtr<UMeiDouComboData> ComboData;

	UPROPERTY(EditDefaultsOnly, Category="MeiDou")
	TObjectPtr<UMeiDouPoseSet> PoseSet;

	// a partial combo (1 or 2 poses) is dropped after this long without a new input
	UPROPERTY(EditDefaultsOnly, Category="MeiDou")
	float ComboInputTimeoutSeconds = 2.f;

	UPROPERTY()
	TObjectPtr<ACharacter> OwnerCharacter;

	UPROPERTY()
	TObjectPtr<UAttackComponent> AttackComponent;

	UPROPERTY()
	TObjectPtr<ULockOnComponent> LockOnComponent;

	UPROPERTY(VisibleAnywhere, Category="MeiDou")
	EMeiDouState State = EMeiDouState::EMDS_Idle;

	// poses of the current combo, the playing one included
	TArray<EMeiDouInput> ComboInputBuffer;

	// one input pressed during a pose, processed when that pose reaches ActionEnd
	EMeiDouInput QueuedInput = EMeiDouInput::None;

	// repeating the previous pose alternates mirrored / normal
	bool bLastPoseMirrored = false;

	// the combo whose result is playing
	FMeiDouCombo ActiveCombo;

	// montage that owns the current action
	UPROPERTY()
	TObjectPtr<UAnimMontage> CurrentMontage;

	// engine id of the currently playing montage instance
	int32 CurrentInstanceId = INDEX_NONE;

	FTimerHandle ComboInputTimeoutHandle;

	// actor spawned by the playing result, told when the result ends
	TWeakObjectPtr<AMeiDouResult> SpawnedResult;
};
