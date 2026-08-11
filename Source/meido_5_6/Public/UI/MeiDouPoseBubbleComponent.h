// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "Types/MeiDouTypes.h"
#include "MeiDouPoseBubbleComponent.generated.h"

class UMeiDouComponent;
class UMeiDouPoseBubbleWidget;
struct FMeiDouPoseAnimationRequest;

/**
 * Player-only world bubble for MeiDou poses (CP3.5).
 * Attach to player maid root; assign BubbleWidgetClass (WBP child of MeiDouPoseBubbleWidget).
 * Listens to owner's UMeiDouComponent — not for enemies.
 */
UCLASS(ClassGroup = (UI), meta = (BlueprintSpawnableComponent))
class MEIDO_5_6_API UMeiDouPoseBubbleComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:
	UMeiDouPoseBubbleComponent();

	UPROPERTY(EditAnywhere, Category = "MeiDou|Bubble")
	TSubclassOf<UMeiDouPoseBubbleWidget> BubbleWidgetClass;

	UPROPERTY(EditAnywhere, Category = "MeiDou|Bubble", meta = (ClampMin = "0.1"))
	float DisplayDuration = 2.f;

	/** If true, bind owner's MeiDou in BeginPlay. */
	UPROPERTY(EditAnywhere, Category = "MeiDou|Bubble")
	bool bBindToOwnerMeiDouOnBeginPlay = true;

	UFUNCTION(BlueprintCallable, Category = "MeiDou|Bubble")
	void ShowPoseBubble(EMeiDouInput Input);

	UFUNCTION(BlueprintCallable, Category = "MeiDou|Bubble")
	void HidePoseBubble();

	UFUNCTION(BlueprintCallable, Category = "MeiDou|Bubble")
	void BindToMeiDou(UMeiDouComponent* MeiDou);

	UFUNCTION(BlueprintCallable, Category = "MeiDou|Bubble")
	void UnbindMeiDou();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void HandlePoseRequested(const FMeiDouPoseAnimationRequest& Request);

	UPROPERTY(Transient)
	TObjectPtr<UMeiDouComponent> BoundMeiDou = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UMeiDouPoseBubbleWidget> BubbleWidget = nullptr;

	FTimerHandle HideTimerHandle;

	void EnsureBubbleWidget();
	void ClearHideTimer();
};
