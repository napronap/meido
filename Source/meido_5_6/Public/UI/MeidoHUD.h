// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MeidoHUD.generated.h"

class UProgressBar;
class UHealthComponent;

/**
 * Gameplay HUD root (screen-space). Logic lives here; WBP child is presentation only.
 *
 * Author: create WBP parented to MeidoHUD. Optional BindWidget name "HealthBar" (ProgressBar).
 * Mounted by AMeidoPlayerController — not by the pawn.
 */
UCLASS(Blueprintable, Abstract)
class MEIDO_5_6_API UMeidoHUD : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Bind to a health component (usually the possessed pawn's). Replaces any prior bind. */
	UFUNCTION(BlueprintCallable, Category = "HUD|Health")
	void BindToHealth(UHealthComponent* Health);

	UFUNCTION(BlueprintCallable, Category = "HUD|Health")
	void UnbindHealth();

	UFUNCTION(BlueprintCallable, Category = "HUD|Health")
	void SetHealthPercent(float Percent);

protected:
	/** Optional. Author WBP: name a ProgressBar "HealthBar". */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> HealthBar;

	UFUNCTION()
	void HandleHealthChanged(
		UHealthComponent* HealthComponent,
		float CurrentHealth,
		float MaxHealth,
		float Delta
	);

	void ApplyPercentFromHealth(float CurrentHealth, float MaxHealth);

	UPROPERTY(Transient)
	TObjectPtr<UHealthComponent> BoundHealth = nullptr;
};
