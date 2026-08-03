// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CombatAudioData.generated.h"

class USoundBase;

/**
 * Shared combat SFX set (CP1.6).
 * Multiple maids point at the same DA; null slots = silence.
 * Author: assign SoundCue / SoundWave when assets exist.
 */
UCLASS(BlueprintType)
class MEIDO_5_6_API UCombatAudioData : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Swing / whoosh when a hit window opens (or notify later). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|SFX")
	TObjectPtr<USoundBase> MeleeSwing = nullptr;

	/** Confirmed melee hit on a target. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|SFX")
	TObjectPtr<USoundBase> MeleeHitConfirm = nullptr;

	/** This character took damage (hurt). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|SFX")
	TObjectPtr<USoundBase> Hurt = nullptr;

	/** This character died. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|SFX")
	TObjectPtr<USoundBase> Death = nullptr;
};
