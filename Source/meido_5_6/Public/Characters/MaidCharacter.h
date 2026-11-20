#pragma once

#include "CoreMinimal.h"
#include "ActorComponents/MeiDouComponent.h"
#include "MeiDouPoseDataAsset.h"
#include "GameFramework/Character.h"
#include "Types/MeiDouTypes.h"
#include "MaidCharacter.generated.h"

class UAttackComponent;
class UDashComponent;
class UHealthComponent;
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

protected:
	virtual void BeginPlay() override;

	/*
	 *	movement (maybe put camera in player maid class because AI maid won't use it
	 */
	void DoLook(float Yaw, float Pitch);
	virtual void Jump() override;

	/*
	 * Combat
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat")
	TObjectPtr<UAttackComponent> AttackComponent;

	// created by APlayerMaidCharacter (null on enemies); moves to the player class in slice C
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Movement")
	TObjectPtr<UDashComponent> DashComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Health")
	TObjectPtr<UHealthComponent> HealthComponent;

	FOnMontageEnded OnDamageMontageEnded;
	void DamageMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	FOnMontageEnded OnMeiDouFailMontageEnded;
	void MeiDouFailMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	/*
	 * mei dou
	 */
	// created by APlayerMaidCharacter (null on enemies); moves to the player class in slice D
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="MeiDou")
	TObjectPtr<UMeiDouComponent> MeiDouComponent;
	
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

	// true while the fail montage plays (stunned: can't act or take damage). Moves into MeiDouComponent in slice D
	bool bMeiDouFailed = false;

	/*
	 * damage
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Damage")
	UAnimMontage* DamageMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Damage")
	TArray<FName> DamageSectionNames;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|IFrames", meta=(ClampMin="0.0", UIMin="0.0"))
	float PostDamageIFrameDuration = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|IFrames", meta=(ClampMin="0.0", UIMin="0.0"))
	float DashIFrameDuration = 0.10f;

	// called from TakeDamage once health was actually removed
	void PlayHitReaction();
	void Die();

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

	bool bHasDied = false;
	float InvulnerableUntilTime = 0.f;

public:
	/*
	 * state queries: each one asks the component that owns that state
	 */
	bool IsAttacking() const;
	bool IsDashing() const;
	bool IsMeiDouActive() const;
	bool IsHitReacting() const { return bDamageReactionActive; }
	bool IsDead() const;

	// free to start a new action (jump, dash): nothing else is going on
	bool CanAct() const;

	virtual void ResetForFlowRestart();

protected:
	void GrantIFrames(float DurationSeconds);
	bool HasActiveIFrames() const;

private:
	// dead, reacting to a hit, stunned by a MeiDou fail or in the air: no action can start
	bool IsLocked() const;

	// tells the anim instance whether to mirror the current MeiDou pose
	void SetMirrorFlag(bool bShouldMirror) const;
};
