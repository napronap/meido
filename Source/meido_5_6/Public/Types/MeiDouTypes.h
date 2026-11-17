#pragma once

#include "CoreMinimal.h"
#include "MeiDouTypes.generated.h"

class AActor;
class UAnimMontage;

// simple types to figure out in what state the component is
UENUM(BlueprintType)
enum class EMeiDouState : uint8
{
	EMDS_Idle UMETA(DisplayName = "Idle"),
	EMDS_Posing UMETA(DisplayName = "Posing"),
	EMDS_Finished UMETA(DisplayName = "Result"),
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

// spawn-type resolutions need this config to figure out what and how to spawn it
USTRUCT(BlueprintType)
struct FMeiDouSpawnConfig
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
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

// key for the value of the combo
// A B and C represent the inputs in order
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

	// overload the == operator when comparing two structs of type FMeiDouComboKey
	bool operator==(const FMeiDouComboKey& Other) const
	{
		return A == Other.A && B == Other.B && C == Other.C;
	}
};

// overrides the hashing of keys of type FMeiDouComboKey so when you try to index a TMap with a key
// that is of type FMeiDouComboKey& it will use this function to translate it
FORCEINLINE uint32 GetTypeHash(const FMeiDouComboKey& Key)
{
	return HashCombine(
		HashCombine(::GetTypeHash(Key.A), ::GetTypeHash(Key.B)),
		::GetTypeHash(Key.C)
	);
}

// the real value of the combo. can have information about the combo like damage, usage of resources, etc
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
