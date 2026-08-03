#pragma once

#include "CoreMinimal.h"
#include "ActorComponents/MeiDouComponent.h"
#include "MeiDouPoseDataAsset.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/Character.h"
#include "Types/MeiDouTypes.h"
#include "MaidCharacter.generated.h"

class UAttackComponent;
class UCameraShakeBase;
class UCharacterStateComponent;
class UDashComponent;
class UHealthComponent;
class ULockOnComponent;
class UAnimMontage;
class AController;

UCLASS(Abstract)
class MEIDO_5_6_API AMaidCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AMaidCharacter();

	virtual void Tick(float DeltaTime) override;
	virtual float TakeDamage(
		float DamageAmount,
		struct FDamageEvent const& DamageEvent,
		AController* EventInstigator,
		AActor* DamageCauser
	) override;

	/*
	 * movement/combat actions (player or AI)
	 */
	void DoMove(float Right, float Forward);
	void DoStartComboAttack();
	bool DoDash(const FVector2D& MoveInput = FVector2D::ZeroVector, bool bLockOnActive = false);

	/** Used by UAttackComponent combo chain to bind montage end. */
	FOnMontageEnded OnAttackMontageEnded;

	/*
	 * State — Apply* helpers write CharacterStateComponent slices.
	 */
	void ApplyGameplayStateIdle();
	void ApplyGameplayStateAttacking();
	void ApplyGameplayStateWhiffRecover();
	void ApplyGameplayStateStagger();
	void ApplyGameplayStateJumping();
	void ApplyGameplayStateDashing();
	void ApplyGameplayStateMeiDouActive();
	void ApplyGameplayStateMeiDouFailed();
	void ApplyGameplayStateDead();

protected:
	virtual void BeginPlay() override;

	/*
	 *	movement (maybe put camera in player maid class because AI maid won't use it
	 */
	void DoLook(float Yaw, float Pitch);
	virtual void Jump() override;
	virtual void StopJumping() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State")
	UCharacterStateComponent* CharacterStateComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat")
	UAttackComponent* AttackComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Movement")
	UDashComponent* DashComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Health")
	UHealthComponent* HealthComponent;

	void AttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	FOnMontageEnded OnDamageMontageEnded;
	void DamageMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	FOnMontageEnded OnMeiDouFailMontageEnded;
	void MeiDouFailMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	/*
	 * mei dou
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="MeiDou")
	UMeiDouComponent* MeiDouComponent;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="MeiDou")
	TMap<EMeiDouInput, UMeiDouPoseDataAsset*> MeiDouPoseDataMap;

	void RegisterMeiDouInput(EMeiDouInput Input);

	UFUNCTION()
	void HandleMeiDouPoseAnimationRequested(const FMeiDouPoseAnimationRequest& Request);

	UFUNCTION()
	void HandleMeiDouComboResolved(const FMeiDouResolvedCombo& Result);

	UFUNCTION()
	void HandleMeiDouComboFailed(const FMeiDouResolvedCombo& Result);

	UFUNCTION()
	void HandleMeiDouControlLockChanged(bool bIsLocked);

	UPROPERTY(EditDefaultsOnly, Category="MeiDou")
	TMap<EMeiDouInput, UAnimMontage*> PoseMontages;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MeiDou")
	UAnimMontage* MeiDouFailMontage = nullptr;

	/*
	 * damage
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Damage")
	UAnimMontage* DamageMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Damage")
	TArray<FName> DamageSectionNames;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|IFrames", meta=(ClampMin="0.0", UIMin="0.0"))
	float PostDamageIFrameDuration = 0.35f;

	/**
	 * Post-hit i-frames only for player-controlled pawns when true (current feel).
	 * Enemies stay open to multi-hit. Tune / revisit with dash-as-primary invuln later.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|IFrames")
	bool bPostDamageIFramesPlayerOnly = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|IFrames", meta=(ClampMin="0.0", UIMin="0.0"))
	float DashIFrameDuration = 0.10f;

	/** Camera shake when this pawn takes damage (local player). Null = no shake until author assigns. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Feedback|Shake")
	TSubclassOf<UCameraShakeBase> DamageReceivedCameraShake;

	/** Stronger shake on death (optional; falls back to DamageReceivedCameraShake). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Feedback|Shake")
	TSubclassOf<UCameraShakeBase> DeathCameraShake;

	/** Apply short hit-stop on self when receiving damage (stacks with attacker pair feel). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Feedback")
	bool bHitStopOnDamageReceived = true;

	UFUNCTION()
	void HandleDamageTaken(
		UHealthComponent* InHealthComponent,
		float Damage,
		float CurrentHealth,
		AActor* DamageCauser,
		AController* InstigatedBy
	);

	UFUNCTION()
	void HandleHealthDepleted(UHealthComponent* InHealthComponent, AActor* DamageCauser);

	int32 NextDamageSectionIndex = 0;
	bool bDamageReactionActive = false;

	/*
	 * death
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Death")
	UAnimMontage* DeathMontage = nullptr;

	// actor will auto destroy this many seconds after death (used for the enemy maids, player shouldn't destroy for infinite loop)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Death", meta=(ClampMin="0.0", UIMin="0.0"))
	float DeathLifeSpanSeconds = 6.f;

	/** Presentation/flow mirror of Health dead. Prefer IsDead() / HealthComponent->IsDead() for queries. */
	bool bHasDied = false;
	float InvulnerableUntilTime = 0.f;
	/** While true, HasActiveIFrames ignores timer (dash notify window). */
	bool bIFrameOverrideActive = false;

	/*
	 * lock on
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="LockOn")
	ULockOnComponent* LockOnComponent;

public:
	bool CanStartDash() const;
	/** @param bGrantLegacyTimedIFrames player timed i-frames if dash anim has no I-Frame notify yet */
	void NotifyDashStarted(bool bGrantLegacyTimedIFrames = true);
	void NotifyDashEnded();
	bool IsDead() const;
	virtual void ResetForFlowRestart();

	UFUNCTION(BlueprintPure, Category = "State")
	UCharacterStateComponent* GetCharacterStateComponent() const { return CharacterStateComponent; }

	/** AnimNotifyState_DashIFrames — invuln while notify range is active. */
	void SetIFrameOverrideActive(bool bActive);

	void GrantIFrames(float DurationSeconds);
	bool HasActiveIFrames() const;
};
