// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/CombatTeamTypes.h"
#include "AttackComponent.generated.h"

class AMaidCharacter;
class UAnimMontage;
class UCharacterStateComponent;
class UCombatAudioData;
class USoundBase;

UENUM(BlueprintType)
enum class EHitStopType : uint8
{
	Light UMETA(DisplayName = "Light"),
	Heavy UMETA(DisplayName = "Heavy")
};

/**
 * Melee combat on the pawn:
 * - Hit windows (open/close, box sweep, team filter, damage, hit-stop)
 * - Combo chain (montage, buffer follow-up, section jump, recovery)
 *
 * Character owns input/world gates; team identity is ICombatTeamSource on the owner.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MEIDO_5_6_API UAttackComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAttackComponent();

	// --- Hit session / windows ---

	/** Defaults for upcoming hit windows (hit-stop type). Does not open a window. */
	UFUNCTION(BlueprintCallable, Category = "Attack")
	void StartAttack(EHitStopType InHitStopType = EHitStopType::Light);

	UFUNCTION(BlueprintCallable, Category = "Attack")
	void OpenHitWindow(FName InSocket);

	UFUNCTION(BlueprintCallable, Category = "Attack")
	void OpenHitWindowSockets(const TArray<FName>& InSockets);

	/** Close active trace window only. */
	UFUNCTION(BlueprintCallable, Category = "Attack")
	void CloseHitWindow();

	UFUNCTION(BlueprintPure, Category = "Attack")
	bool IsHitWindowOpen() const { return bHitWindowOpen; }

	UFUNCTION(BlueprintPure, Category = "Attack|Team")
	ECombatTeam GetOwnerCombatTeam() const;

	// --- Combo chain ---

	/** Start chain if not mid-combo; otherwise buffer one follow-up press. */
	UFUNCTION(BlueprintCallable, Category = "Attack|Combo")
	void RequestComboAttack();

	/** AnimNotify_CheckCombo */
	UFUNCTION(BlueprintCallable, Category = "Attack|Combo")
	void NotifyCheckCombo();

	/** AnimNotify_RecoveryEnd */
	UFUNCTION(BlueprintCallable, Category = "Attack|Combo")
	void NotifyRecoveryEnd();

	/** Clear step/buffer (montage end, flow reset). */
	UFUNCTION(BlueprintCallable, Category = "Attack|Combo")
	void ResetComboRuntime();

	UFUNCTION(BlueprintPure, Category = "Attack|Combo")
	bool IsComboInputBuffered() const { return bComboInputBuffered; }

	UFUNCTION(BlueprintPure, Category = "Attack|Combo")
	int32 GetComboStepIndex() const { return ComboStepIndex; }

	// --- Combo data ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack|Combo")
	TObjectPtr<UAnimMontage> ComboAttackMontage = nullptr;

	/** Section names in order; [0] is first hit. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack|Combo")
	TArray<FName> ComboSectionNames;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack|Combo", meta = (ClampMin = "0.0"))
	float RecoveryBlendOutTime = 0.3f;

	/**
	 * Shared combat SFX (CP1.6). Same DA on player/enemy BPs.
	 * Null DA or null slots = silence. Author fills SoundCues later.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack|Audio")
	TObjectPtr<UCombatAudioData> CombatAudio = nullptr;

	UFUNCTION(BlueprintPure, Category = "Attack|Audio")
	UCombatAudioData* GetCombatAudio() const { return CombatAudio; }

	// --- Hit trace / damage ---

	UPROPERTY(EditAnywhere, Category = "Attack")
	float Damage = 20.f;

	UPROPERTY(EditAnywhere, Category = "Attack")
	FVector BoxExtent = FVector(20.f, 20.f, 20.f);

	UPROPERTY(EditAnywhere, Category = "Attack")
	float TraceDistance = 60.f;

	UPROPERTY(EditAnywhere, Category = "Attack|Team")
	bool bAllowFriendlyFire = false;

	UPROPERTY(EditAnywhere, Category = "Attack")
	FName HitSocketName = "Attack_Hand_R";

	UPROPERTY(EditAnywhere, Category = "Attack|Debug")
	bool bDrawDebugHitTrace = false;

	UPROPERTY(EditAnywhere, Category = "Attack|HitStop", meta = (ClampMin = "0.001", ClampMax = "1.0"))
	float HitStopTimeDilation = 0.01f;

	UPROPERTY(EditAnywhere, Category = "Attack|HitStop", meta = (ClampMin = "0.0", ClampMax = "0.2"))
	float LightHitStopDuration = 0.04f;

	UPROPERTY(EditAnywhere, Category = "Attack|HitStop", meta = (ClampMin = "0.0", ClampMax = "0.3"))
	float HeavyHitStopDuration = 0.08f;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction
	) override;

	void PerformHitTrace();
	bool ShouldIgnoreHitActor(const AActor* HitActor) const;
	float GetHitStopDuration(EHitStopType HitStopType) const;
	void ApplyHitStopForHit(AActor* HitActor);
	void ApplyDamageToHit(AActor* HitActor);
	void PlayCombatSfx(USoundBase* Sound) const;
	static ECombatTeam ResolveCombatTeam(const AActor* Actor);

	void StartComboChain();
	bool IsInActiveComboAttack() const;
	AMaidCharacter* GetMaidOwner() const;
	UCharacterStateComponent* ResolveStateComponent() const;

	UPROPERTY()
	TObjectPtr<ACharacter> OwnerCharacter;

	bool bHitWindowOpen = false;
	EHitStopType CurrentHitStopType = EHitStopType::Light;
	TSet<AActor*> HitActorsThisWindow;
	TArray<FName> CurrentHitSockets;

	int32 ComboStepIndex = 0;
	bool bComboInputBuffered = false;
};
