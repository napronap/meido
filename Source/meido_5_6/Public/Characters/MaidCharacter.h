#pragma once

#include "CoreMinimal.h"
#include "ActorComponents/MeiDouComponent.h"
#include "MeiDouPoseDataAsset.h"
#include "GameFramework/Character.h"
#include "Types/CharacterTypes.h"
#include "Interfaces/ComboAttacker.h"
#include "Types/MeiDouTypes.h"
#include "MaidCharacter.generated.h"

class UAttackComponent;
class UDashComponent;
class UHealthComponent;
class ULockOnComponent;
class UAnimMontage;
class AController;

UCLASS(Abstract)
class MEIDO_5_6_API AMaidCharacter : public ACharacter, public IComboAttacker
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
	virtual void StopJumping() override;

	/*
	 * State
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State")
	ECharacterState CharacterState = ECharacterState::ECS_Idle;

	/*
	 * Combat
	 */
	UPROPERTY(EditAnywhere, Category="Combat|Combo")
	UAnimMontage* ComboAttackMontage;

	UPROPERTY(EditAnywhere, Category="Combat|Combo")
	TArray<FName> ComboSectionNames;

	int32 ComboIndex = 0;
	int32 ComboCount = 0;
	float CachedAttackInputTime = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat")
	UAttackComponent* AttackComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Movement")
	UDashComponent* DashComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Health")
	UHealthComponent* HealthComponent;

	void DoContinueCombo();

	virtual void CheckCombo_Implementation() override;
	virtual void RecoveryEnd_Implementation() override;

	FOnMontageEnded OnAttackMontageEnded;
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|IFrames", meta=(ClampMin="0.0", UIMin="0.0"))
	float DashIFrameDuration = 0.10f;

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

	// If > 0, actor will auto-destroy this many seconds after death.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Death", meta=(ClampMin="0.0", UIMin="0.0"))
	float DeathLifeSpanSeconds = 6.f;

	bool bHasDied = false;
	float InvulnerableUntilTime = 0.f;

	/*
	 * lock on
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="LockOn")
	ULockOnComponent* LockOnComponent;

public:
	bool CanStartDash() const;
	void NotifyDashStarted();
	void NotifyDashEnded();
	ECharacterState GetCharacterState() const;
	bool IsDead() const;

protected:
	void GrantIFrames(float DurationSeconds);
	bool HasActiveIFrames() const;
};
