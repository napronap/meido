#pragma once

#include "CoreMinimal.h"
#include "MeiDouTypes.generated.h"

class AActor;

// what the MeiDou component is doing
UENUM(BlueprintType)
enum class EMeiDouState : uint8
{
	// free (a partial combo may be waiting for its next input)
	EMDS_Idle	UMETA(DisplayName = "Idle"),
	EMDS_Posing UMETA(DisplayName = "Posing"),
	EMDS_Result UMETA(DisplayName = "Result"),
	EMDS_Failed UMETA(DisplayName = "Failed"),
};

// the individual poses
UENUM(BlueprintType)
enum class EMeiDouInput : uint8
{
	EMDI_Moe	UMETA(DisplayName = "Moe"),
	EMDI_Kyun	UMETA(DisplayName = "Kyun"),
	EMDI_Nyan	UMETA(DisplayName = "Nyan"),

	// for initialization purposes
	None		UMETA(DisplayName = "None"),
};

// result type of the combo
// spawn: spawns an actor
// damage: performs traces on the animation
UENUM(BlueprintType)
enum class EMeiDouResultType : uint8
{
	EMDRT_Spawn	 UMETA(DisplayName = "Spawn"),
	EMDRT_Damage UMETA(DisplayName = "Damage"),
};

// whether a spawner type result spawns on target or on the emitting actor
UENUM(BlueprintType)
enum class EMeiDouResultSpawnLocation : uint8
{
	EMDRSL_Target		 UMETA(DisplayName = "Target"),
	EMDRSL_EmittingActor UMETA(DisplayName = "EmittingActor"),
};

// Valid events for the anim notify. MeiDouComponent acts accordingly to these
UENUM(BlueprintType)
enum class EMeiDouAnimEvent : uint8
{
	EMDAE_Spawn		 UMETA(DisplayName = "Spawn"),
	EMDAE_TraceStart UMETA(DisplayName = "TraceStart"),
	EMDAE_TraceEnd	 UMETA(DisplayName = "TraceEnd"),
	// pose / result done: the next pose can chain (like the attack's RecoveryEnd). The rest of the montage is cosmetic
	EMDAE_ActionEnd	 UMETA(DisplayName = "ActionEnd"),

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
