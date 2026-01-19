#pragma once

#include "CoreMinimal.h"
#include "MeiDouTypes.generated.h"

UENUM(BlueprintType)
enum class EMeiDouInput : uint8
{
	EMDI_Moe UMETA(DisplayName = "Moe"),
	EMDI_Kyun UMETA(DisplayName = "Kyun"),
	EMDI_Nyan UMETA(DisplayName = "Nyan"),
};

// key for the value of the combo
// A B and C represent the inputs in order
USTRUCT(BlueprintType)
struct FMeiDouComboKey
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	EMeiDouInput A;

	UPROPERTY(EditAnywhere)
	EMeiDouInput B;

	UPROPERTY(EditAnywhere)
	EMeiDouInput C;

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
};