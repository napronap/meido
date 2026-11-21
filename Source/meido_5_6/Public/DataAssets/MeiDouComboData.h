#pragma once

#include "CoreMinimal.h"
#include "DataAssets/AttackComboData.h"
#include "Engine/DataAsset.h"
#include "Types/MeiDouTypes.h"
#include "MeiDouComboData.generated.h"

class UAnimMontage;

// the successful combo output after inputting a valid combination
USTRUCT(BlueprintType)
struct FMeiDouCombo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName Id;

	// this is compared in order against the input buffer
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<EMeiDouInput> Inputs;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> Montage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EMeiDouResultType ResultType = EMeiDouResultType::EMDRT_Spawn;

	// a summon i.e oishikunare or maid beam
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition="ResultType == EMeiDouResultType::EMDRT_Spawn", EditConditionHides))
	FMeiDouSpawnConfig Spawn;

	// a "normal" hit with hitboxes
	// montage needs TraceStart / TraceEnd events
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition="ResultType == EMeiDouResultType::EMDRT_Damage", EditConditionHides))
	FAttackHit Hit;
};


// every MeiDou combo the character can resolve
// any combination not listed here is a fail
UCLASS()
class MEIDO_5_6_API UMeiDouComboData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FMeiDouCombo> Combos;
};
