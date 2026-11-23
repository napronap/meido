#pragma once

#include "CoreMinimal.h"
#include "ActorComponents/MeiDouComponent.h"
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

	// movement/combat actions (player or AI), each one gates itself
	void DoMove(float Right, float Forward);
	// AimInput is the move input, not camera input !!
	void DoStartComboAttack(const FVector2D& AimInput = FVector2D::ZeroVector);
	bool DoDash(const FVector2D& MoveInput = FVector2D::ZeroVector, bool bLockOnActive = false);
	virtual void Jump() override;
	void RegisterMeiDouInput(EMeiDouInput Input);

protected:
	virtual void BeginPlay() override;

	// Combat
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat")
	TObjectPtr<UAttackComponent> AttackComponent;

	// created by APlayerMaidCharacter (null on enemies)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Movement")
	TObjectPtr<UDashComponent> DashComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Health")
	TObjectPtr<UHealthComponent> HealthComponent;

	FOnMontageEnded OnDamageMontageEnded;
	void DamageMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	// MeiDou
	// created by APlayerMaidCharacter (null on enemies)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="MeiDou")
	TObjectPtr<UMeiDouComponent> MeiDouComponent;

	// Damage receiving
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Damage")
	UAnimMontage* DamageMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Damage")
	TArray<FName> DamageSectionNames;

	// TODO: remove, should be driven by the montage
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|IFrames", meta=(ClampMin="0.0", UIMin="0.0"))
	float PostDamageIFrameDuration = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|IFrames", meta=(ClampMin="0.0", UIMin="0.0"))
	float DashIFrameDuration = 0.10f;

	// called from TakeDamage once health was actually removed
	void PlayHitReaction();
	void Die();

	int32 NextDamageSectionIndex = 0;
	bool bDamageReactionActive = false;

	// Death
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Death")
	UAnimMontage* DeathMontage = nullptr;

	// actor will auto destroy this many seconds after death (used for the enemy maids, player shouldn't destroy for infinite loop)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Death", meta=(ClampMin="0.0", UIMin="0.0"))
	float DeathLifeSpanSeconds = 6.f;

	bool bHasDied = false;
	float InvulnerableUntilTime = 0.f;

public:
	// state queries, always ask the owning component
	bool IsAttacking() const;
	bool IsDashing() const;
	bool IsMeiDouActive() const;
	// stunned by a failed MeiDou combo (can't act, takes no damage)
	// TODO: maybe take damage you know
	bool IsMeiDouFailed() const;
	bool IsHitReacting() const { return bDamageReactionActive; }
	bool IsDead() const;

	// free to start a new action (jump, dash)
	bool CanAct() const;

	// for external listeners of OnHealthChanged/OnHealthDepleted (GameMode)
	UHealthComponent* GetHealthComponent() const { return HealthComponent; }

	// for the HUD's MeiDou indicator
	UMeiDouComponent* GetMeiDouComponent() const { return MeiDouComponent; }

	// this method is exclusive for use in the GM_Meido (demo)
	// TODO: remove at some point
	virtual void ResetForFlowRestart();

protected:
	void GrantIFrames(float DurationSeconds);
	bool HasActiveIFrames() const;

private:
	// dead, reacting to a hit, stunned by a MeiDou fail or in the air
	bool IsLocked() const;
};
