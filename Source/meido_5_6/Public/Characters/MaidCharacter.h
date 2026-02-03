#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Types/CharacterTypes.h"
#include "Interfaces/ComboAttacker.h"
#include "Types/MeiDouTypes.h"
#include "MaidCharacter.generated.h"

class UAttackComponent;
class UMeiDouComponent;
class ULockOnComponent;
class UAnimMontage;

UCLASS(Abstract)
class MEIDO_5_6_API AMaidCharacter : public ACharacter, public IComboAttacker
{
	GENERATED_BODY()

public:
	AMaidCharacter();

	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	/* =========================
	 *  CORE MOVEMENT / LOOK
	 * ========================= */
	void DoMove(float Right, float Forward);
	void DoLook(float Yaw, float Pitch);
	virtual void Jump() override;
	virtual void StopJumping() override;

	/* =========================
	 *  STATE
	 * ========================= */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State")
	ECharacterState CharacterState = ECharacterState::ECS_Idle;

	/* =========================
	 *  COMBAT
	 * ========================= */
	UPROPERTY(EditAnywhere, Category="Combat|Combo")
	UAnimMontage* ComboAttackMontage;

	UPROPERTY(EditAnywhere, Category="Combat|Combo")
	TArray<FName> ComboSectionNames;

	int32 ComboIndex = 0;
	int32 ComboCount = 0;
	float CachedAttackInputTime = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat")
	UAttackComponent* AttackComponent;

	void DoStartComboAttack();
	void DoContinueCombo();

	virtual void CheckCombo_Implementation() override;
	virtual void RecoveryEnd_Implementation() override;

	FOnMontageEnded OnAttackMontageEnded;
	void AttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	/* =========================
	 *  MEIDOU SYSTEM
	 * ========================= */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="MeiDou")
	UMeiDouComponent* MeiDouComponent;

	void RegisterMeiDouInput(EMeiDouInput Input);

	UFUNCTION()
	void HandleMeiDouComboResolved(const FMeiDouResolvedCombo& Result);

	UPROPERTY(EditDefaultsOnly, Category="MeiDou")
	TMap<EMeiDouInput, UAnimMontage*> PoseMontages;

	/* =========================
	 *  LOCK ON (LOGIC ONLY)
	 * ========================= */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="LockOn")
	ULockOnComponent* LockOnComponent;
};
