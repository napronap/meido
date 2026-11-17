// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "MaidPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class UUserWidget;
class AEnemyMaid;
class UMeidoHUD;

/**
 * Playground player controller (no demo flow).
 * CP4.2+: local pause with optional pause widget.
 */
UCLASS()
class MEIDO_5_6_API AMaidPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultIMC;

	/** Same pause IA as demo if shared (map Escape / Start). */
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> PauseInputAction;

	/** Optional. If null, pause still freezes the game (no UI). */
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> PauseMenuWidgetClass;

	UPROPERTY(EditAnywhere, Category = "UI|HUD")
	TSubclassOf<UMeidoHUD> GameplayHUDClass;

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	UFUNCTION(BlueprintCallable, Category = "Pause")
	void TogglePauseMenu();

	UFUNCTION(BlueprintCallable, Category = "Pause")
	void ResumeFromPause();

	UFUNCTION(BlueprintPure, Category = "Pause")
	bool IsPlaygroundPaused() const { return bPlaygroundPaused; }

private:
	UPROPERTY(Transient)
	TObjectPtr<UMeidoHUD> GameplayHUD;
	void RefreshGameplayHUD();
	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> PauseMenuWidget = nullptr;

	bool bPlaygroundPaused = false;

	// Debug: K freezes/unfreezes enemy AI — POST 4.b
	bool bDebugDisableEnemyAI = false;

	void OnPauseInput(const FInputActionValue& Value);
	void OnDebugToggleEnemyAI();
	void SetAllEnemyAIEnabled(bool bEnabled);
	void SetEnemyAIEnabled(AEnemyMaid* Enemy, bool bEnabled);
	void EnterPaused();
	void LeavePaused();
};
