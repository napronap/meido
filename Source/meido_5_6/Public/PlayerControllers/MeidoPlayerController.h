// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MeidoPlayerController.generated.h"

class UInputMappingContext;
class UMeidoHUD;

/**
 * Main product PlayerController: owns local UI mount (HUD) + input mapping.
 * Separate from AMaidPlayerController (playground) and ADemoMaidPlayerController (legacy flow).
 * Flow/menus/win-lose come later; this CP is HUD scaffold only.
 */
UCLASS()
class MEIDO_5_6_API AMeidoPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	/** Create (if needed) and add HUD to viewport; rebind health to current pawn. */
	UFUNCTION(BlueprintCallable, Category = "UI|HUD")
	void ShowMeidoHUD();

	UFUNCTION(BlueprintCallable, Category = "UI|HUD")
	void HideMeidoHUD();

protected:
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultIMC;

	/** WBP child of UMeidoHUD (e.g. WBP_MeidoHUD). Assign on BP_MeidoPlayerController. */
	UPROPERTY(EditAnywhere, Category = "UI|HUD")
	TSubclassOf<UMeidoHUD> MeidoHUDClass;

	UPROPERTY(EditAnywhere, Category = "UI|HUD", meta = (ClampMin = "0"))
	int32 MeidoHUDZOrder = 0;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "UI|HUD")
	TObjectPtr<UMeidoHUD> MeidoHUD = nullptr;

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	void BindHUDToPawn(APawn* InPawn);
};
