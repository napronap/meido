// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SimpleHealthBar.generated.h"

class UProgressBar;

/**
 * Display-only. Author WBP: parent this class; name a ProgressBar "HealthBar" (BindWidget).
 * No logic — USimpleHealthBarComponent drives the percent.
 *
 * Keep the HealthBar property layout stable (TObjectPtr). Changing it invalidates
 * any map/BP that already serialized a SimpleHealthBar instance.
 */
UCLASS(Blueprintable)
class MEIDO_5_6_API USimpleHealthBar : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UProgressBar> HealthBar;
};
