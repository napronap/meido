// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FlowTypes.generated.h"

/** Demo run flow (CP4.1). Owned by AMeidoDemoGameMode. */
UENUM(BlueprintType)
enum class EFlowState : uint8
{
	MainMenu UMETA(DisplayName = "MainMenu"),
	/** First-run tutorial; gameplay begins after its final slide. */
	Tutorial UMETA(DisplayName = "Tutorial"),
	Playing UMETA(DisplayName = "Playing"),
	Paused UMETA(DisplayName = "Paused"),
	Win UMETA(DisplayName = "Win"),
	Lose UMETA(DisplayName = "Lose"),
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFlowStateChanged, EFlowState, NewState);
