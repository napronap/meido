// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "MaidAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class MEIDO_5_6_API UMaidAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Maid")
	bool bShouldMirror = false;

	UPROPERTY(BlueprintReadOnly, Category="Maid|Movement")
	bool bIsLockedOn = false;
};
