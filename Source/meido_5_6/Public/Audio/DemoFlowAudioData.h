#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DemoFlowAudioData.generated.h"

class USoundBase;

/** UI and flow sounds only. Combat continues to use UCombatAudioData. */
UCLASS(BlueprintType)
class MEIDO_5_6_API UDemoFlowAudioData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<USoundBase> Navigate = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<USoundBase> Confirm = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<USoundBase> Cancel = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flow")
	TObjectPtr<USoundBase> Pause = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flow")
	TObjectPtr<USoundBase> Resume = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flow")
	TObjectPtr<USoundBase> Win = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flow")
	TObjectPtr<USoundBase> Lose = nullptr;
};
