// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Types/MeiDouTypes.h"
#include "ActorComponents/MeiDouComponent.h"
#include "MeidoHUD.generated.h"

class UProgressBar;
class UTextBlock;
class UHealthComponent;
class UMeiDouComponent;

/**
 * Gameplay HUD root (screen-space). Logic here; WBP = presentation.
 *
 * Author: WBP parent MeidoHUD.
 * - Health: ProgressBar "HealthBar"
 * - MeiDou queue (CP3.5): optional TextBlocks "MeiDouSlot0/1/2" or use OnMeiDouQueueUpdated in BP
 * Mounted by AMeidoPlayerController.
 */
UCLASS(Blueprintable, Abstract)
class MEIDO_5_6_API UMeidoHUD : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "HUD|Health")
	void BindToHealth(UHealthComponent* Health);

	UFUNCTION(BlueprintCallable, Category = "HUD|Health")
	void UnbindHealth();

	UFUNCTION(BlueprintCallable, Category = "HUD|Health")
	void SetHealthPercent(float Percent);

	UFUNCTION(BlueprintCallable, Category = "HUD|MeiDou")
	void BindToMeiDou(UMeiDouComponent* MeiDou);

	UFUNCTION(BlueprintCallable, Category = "HUD|MeiDou")
	void UnbindMeiDou();

	/** Refresh queue UI from buffer (also called on OnMeiDouBufferChanged). */
	UFUNCTION(BlueprintCallable, Category = "HUD|MeiDou")
	void ApplyMeiDouBuffer(
		const TArray<EMeiDouInput>& Buffer,
		EMeiDouBufferChangeReason Reason
	);

	/** BP: custom icons / flash on drop. C++ fills optional slot text if bound. */
	UFUNCTION(BlueprintNativeEvent, Category = "HUD|MeiDou")
	void OnMeiDouQueueUpdated(
		const TArray<EMeiDouInput>& Buffer,
		EMeiDouBufferChangeReason Reason
	);
	virtual void OnMeiDouQueueUpdated_Implementation(
		const TArray<EMeiDouInput>& Buffer,
		EMeiDouBufferChangeReason Reason
	);

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> HealthBar;

	/** Optional labels for 3 queue slots (Moe/Kyun/Nyan text). */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> MeiDouSlot0;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> MeiDouSlot1;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> MeiDouSlot2;

	UFUNCTION()
	void HandleHealthChanged(
		UHealthComponent* HealthComponent,
		float CurrentHealth,
		float MaxHealth,
		float Delta
	);

	UFUNCTION()
	void HandleMeiDouBufferChanged(
		const TArray<EMeiDouInput>& Buffer,
		EMeiDouBufferChangeReason Reason
	);

	void ApplyPercentFromHealth(float CurrentHealth, float MaxHealth);
	static FText LabelForMeiDouInput(EMeiDouInput Input);
	void SetSlotText(UTextBlock* SlotWidget, EMeiDouInput Input, bool bFilled);

	UPROPERTY(Transient)
	TObjectPtr<UHealthComponent> BoundHealth = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UMeiDouComponent> BoundMeiDou = nullptr;
};
