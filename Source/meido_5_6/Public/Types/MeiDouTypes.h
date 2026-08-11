#pragma once

#include "CoreMinimal.h"
#include "MeiDouTypes.generated.h"

class AActor;
class UAnimMontage;

/** Component FSM (CP3.1). Idle → Posing → Result|Fail → Idle. */
UENUM(BlueprintType)
enum class EMeiDouState : uint8
{
	EMDS_Idle UMETA(DisplayName = "Idle"),
	EMDS_Posing UMETA(DisplayName = "Posing"),
	/** Result technique montage / active definition window. Was EMDS_Finished. */
	EMDS_Result UMETA(DisplayName = "Result"),
};

// simple types to figure out in what state the result ability is
UENUM(BlueprintType)
enum class EMeiDouResultState : uint8
{
	EMDRS_Idle UMETA(DisplayName = "Idle"),
	EMDRS_Active UMETA(DisplayName = "Active"),
	EMDRS_Finished UMETA(DisplayName = "Finished"),
};

UENUM(BlueprintType)
enum class EMeiDouInput : uint8
{
	EMDI_Moe UMETA(DisplayName = "Moe"),
	EMDI_Kyun UMETA(DisplayName = "Kyun"),
	EMDI_Nyan UMETA(DisplayName = "Nyan"),

	// for initialization purposes
	None UMETA(DisplayName = "None"),
};

// result type of the combo
// spawn: spawns an actor
// damage: performs traces on the animation
UENUM(BlueprintType)
enum class EMeiDouResultType : uint8
{
	EMDRT_Spawn UMETA(DisplayName = "Spawn"),
	EMDRT_Damage UMETA(DisplayName = "Damage"),
};

// whether a spawner type result spawns on target or on the emitting actor
UENUM(BlueprintType)
enum class EMeiDouResultSpawnLocation : uint8
{
	EMDRSL_Target UMETA(DisplayName = "Target"),
	EMDRSL_EmittingActor UMETA(DisplayName = "EmittingActor"),
};

// Valid events for the anim notify. MeiDouComponent acts accordingly to these
UENUM(BlueprintType)
enum class EMeiDouAnimEvent : uint8
{
	EMDAE_Spawn UMETA(DisplayName = "Spawn"),
	EMDAE_ControlEnable UMETA(DisplayName = "ControlEnable"),
	EMDAE_ControlDisable UMETA(DisplayName = "ControlDisable"),
	EMDAE_TraceStart UMETA(DisplayName = "TraceStart"),
	EMDAE_TraceEnd UMETA(DisplayName = "TraceEnd"),
	
};

/**
 * Spawn-type result config.
 * CP3.2: prefer SoftSpawnedActorClass + PreloadResults on the component so the first
 * MKN/MMK spawn does not hitch mid-fight. Hard SpawnedActorClass remains as legacy.
 */
USTRUCT(BlueprintType)
struct FMeiDouSpawnConfig
{
	GENERATED_BODY()

	/**
	 * Preferred (CP3.2). Soft path — load via PreloadResults / ResolveSpawnClass,
	 * not the first combat Spawn notify.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MeiDou|Spawn")
	TSoftClassPtr<AActor> SoftSpawnedActorClass;

	/**
	 * Legacy hard ref. Still works; loading the DA may pull the class early.
	 * Prefer Soft + empty hard for hitch control.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MeiDou|Spawn")
	TSubclassOf<AActor> SpawnedActorClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EMeiDouResultSpawnLocation SpawnLocation = EMeiDouResultSpawnLocation::EMDRSL_Target;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition="SpawnLocation == EMeiDouResultSpawnLocation::EMDRSL_Target"))
	float ForwardDistanceIfNoTarget = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition="SpawnLocation == EMeiDouResultSpawnLocation::EMDRSL_EmittingActor"))
	FName EmittingActorSocketName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition="SpawnLocation == EMeiDouResultSpawnLocation::EMDRSL_EmittingActor"))
	FVector SpawnOffset = FVector::ZeroVector;

	// If true, this spawned actor will be told to end and optionally destroyed when PoseActive notify state ends.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bDestroyWhenPoseActiveEnds = false;

	/** Soft (load if needed) first, then hard class. */
	UClass* ResolveSpawnClass() const
	{
		if (!SoftSpawnedActorClass.IsNull())
		{
			if (UClass* Loaded = SoftSpawnedActorClass.Get())
			{
				return Loaded;
			}
			return SoftSpawnedActorClass.LoadSynchronous();
		}
		return SpawnedActorClass.Get();
	}

	/** Force soft class into memory (no-op if only hard ref). */
	void PreloadSpawnClass() const
	{
		if (!SoftSpawnedActorClass.IsNull())
		{
			SoftSpawnedActorClass.LoadSynchronous();
		}
	}
};

// minimal damage config
USTRUCT(BlueprintType)
struct FMeiDouDamageConfig
{
	GENERATED_BODY()

	// Legacy single-socket entry. Still supported as fallback when SocketNames is empty.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName SocketName = NAME_None;

	// Preferred setup: evaluate hit traces from all these sockets during MeiDou damage windows.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FName> SocketNames;
};

/**
 * Legacy TMap key (A/B/C). Prefer FMeiDouRecipe + TArray on the DA (CP3.1).
 * Kept for content that still has Combos map filled.
 */
USTRUCT(BlueprintType)
struct FMeiDouComboKey
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	EMeiDouInput A = EMeiDouInput::None;

	UPROPERTY(EditAnywhere)
	EMeiDouInput B = EMeiDouInput::None;

	UPROPERTY(EditAnywhere)
	EMeiDouInput C = EMeiDouInput::None;

	bool operator==(const FMeiDouComboKey& Other) const
	{
		return A == Other.A && B == Other.B && C == Other.C;
	}
};

FORCEINLINE uint32 GetTypeHash(const FMeiDouComboKey& Key)
{
	return HashCombine(
		HashCombine(::GetTypeHash(Key.A), ::GetTypeHash(Key.B)),
		::GetTypeHash(Key.C)
	);
}

/** Payload of a recipe: result montage + spawn/damage config. */
USTRUCT(BlueprintType)
struct FMeiDouComboDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName ComboId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UAnimMontage* AnimationMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EMeiDouResultType ResultType = EMeiDouResultType::EMDRT_Spawn;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition="ResultType == EMeiDouResultType::EMDRT_Spawn", EditConditionHides))
	FMeiDouSpawnConfig SpawnConfig;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition="ResultType == EMeiDouResultType::EMDRT_Damage", EditConditionHides))
	FMeiDouDamageConfig DamageConfig;
};

/**
 * One castable technique (CP3.1). Editor-friendly list row.
 * Sequence length is 3 for the demo (MMK / MKN / NNN); Find matches full buffer.
 */
USTRUCT(BlueprintType)
struct FMeiDouRecipe
{
	GENERATED_BODY()

	/** Ordered pose inputs (e.g. Moe, Moe, Kyun). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MeiDou")
	TArray<EMeiDouInput> Sequence;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MeiDou")
	FMeiDouComboDefinition Definition;

	bool Matches(const TArray<EMeiDouInput>& Buffer) const
	{
		if (Sequence.Num() != Buffer.Num() || Sequence.Num() == 0)
		{
			return false;
		}
		for (int32 i = 0; i < Sequence.Num(); ++i)
		{
			if (Sequence[i] != Buffer[i])
			{
				return false;
			}
		}
		return true;
	}
};
