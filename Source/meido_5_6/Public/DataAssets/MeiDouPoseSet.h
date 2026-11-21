#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Types/MeiDouTypes.h"
#include "MeiDouPoseSet.generated.h"

class UAnimMontage;

USTRUCT(BlueprintType)
struct FMeiDouPose
{
	GENERATED_BODY()

	// needs to start from its "Pose" section
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> Montage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float PlayRate = 1.5f;
};


// a character's MeiDou animations
UCLASS()
class MEIDO_5_6_API UMeiDouPoseSet : public UDataAsset
{
	GENERATED_BODY()

public:
	const FMeiDouPose& GetPose(EMeiDouInput Input) const
	{
		switch (Input)
		{
		case EMeiDouInput::EMDI_Kyun:
			return Kyun;
		case EMeiDouInput::EMDI_Nyan:
			return Nyan;
		default:
			return Moe;
		}
	}

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FMeiDouPose Moe;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FMeiDouPose Kyun;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FMeiDouPose Nyan;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> FailMontage = nullptr;
};
