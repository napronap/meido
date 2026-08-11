// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Types/MeiDouTypes.h"
#include "MeiDouPoseBubbleWidget.generated.h"

class UTextBlock;

/**
 * Player-only pose speech bubble (CP3.5). Display driven by UMeiDouPoseBubbleComponent.
 * Author WBP: parent this class; optional BindWidget name "PoseText" (TextBlock).
 */
UCLASS(Blueprintable, Abstract)
class MEIDO_5_6_API UMeiDouPoseBubbleWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "MeiDou|Bubble")
	void ShowPose(EMeiDouInput Input);

	UFUNCTION(BlueprintCallable, Category = "MeiDou|Bubble")
	void ShowCustomText(const FText& Text);

	UFUNCTION(BlueprintCallable, Category = "MeiDou|Bubble")
	void HideBubble();

	/** Override in BP for appear/fade anims. Default: SetVisibility. */
	UFUNCTION(BlueprintNativeEvent, Category = "MeiDou|Bubble")
	void OnBubbleShown(EMeiDouInput Input, const FText& DisplayText);
	virtual void OnBubbleShown_Implementation(EMeiDouInput Input, const FText& DisplayText);

	UFUNCTION(BlueprintNativeEvent, Category = "MeiDou|Bubble")
	void OnBubbleHidden();
	virtual void OnBubbleHidden_Implementation();

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> PoseText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MeiDou|Bubble")
	FText MoeText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MeiDou|Bubble")
	FText KyunText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MeiDou|Bubble")
	FText NyanText;

	FText TextForInput(EMeiDouInput Input) const;
};
