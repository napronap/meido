// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MaidPlayerController.generated.h"

class UInputMappingContext;
class AEnemyMaid;

/**
 * Playground player controller (no demo flow / menus).
 */
UCLASS()
class MEIDO_5_6_API AMaidPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	// only using one IMC for now
	UPROPERTY(EditAnywhere, Category = Input)
	UInputMappingContext* DefaultIMC;

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

private:
	// Debug: K freezes/unfreezes enemy AI (same as DemoMaidPlayerController)
	// TODO: remove
	bool bDebugDisableEnemyAI = false;

	void OnDebugToggleEnemyAI();
	void SetAllEnemyAIEnabled(bool bEnabled);
	void SetEnemyAIEnabled(AEnemyMaid* Enemy, bool bEnabled);
};
