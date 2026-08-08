// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "SimpleHealthBarComponent.generated.h"

class UHealthComponent;
class USimpleHealthBar;

/**
 * WidgetComponent that binds the owner's UHealthComponent and drives a USimpleHealthBar.
 * Placement / draw size / space: set on this component in BP.
 * Assign Health Bar Class (WBP child of SimpleHealthBar) — clearer than the generic Widget Class picker.
 */
UCLASS(ClassGroup = (UI), meta = (BlueprintSpawnableComponent))
class MEIDO_5_6_API USimpleHealthBarComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:
	USimpleHealthBarComponent();

	UFUNCTION(BlueprintCallable, Category = "UI|Health")
	void SetHealthPercent(float Percent);

	/**
	 * WBP that parents USimpleHealthBar (e.g. WBP_SimpleHealthBar).
	 * Applied in BeginPlay via SetWidgetClass. Prefer this over the generic Widget Class field.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|Health")
	TSubclassOf<USimpleHealthBar> HealthBarClass;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void BindToOwnerHealth();
	void UnbindHealth();
	void ApplyPercentFromHealth(float CurrentHealth, float MaxHealth);

	UFUNCTION()
	void HandleHealthChanged(
		UHealthComponent* HealthComponent,
		float CurrentHealth,
		float MaxHealth,
		float Delta
	);

	UPROPERTY(Transient)
	TObjectPtr<USimpleHealthBar> HealthBarWidget = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UHealthComponent> BoundHealth = nullptr;
};
