// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MaidCharacter.h"
#include "PlayerMaidCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class ULockOnComponent;

// the AMaidCharacter actions plus dash, MeiDou, lockon and a camera boom
UCLASS()
class MEIDO_5_6_API APlayerMaidCharacter : public AMaidCharacter
{
	GENERATED_BODY()

public:
	APlayerMaidCharacter();

	virtual void ResetForFlowRestart() override;

	USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	ULockOnComponent* GetLockOnComponent() const { return LockOnComponent; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="LockOn")
	TObjectPtr<ULockOnComponent> LockOnComponent;

	UPROPERTY(VisibleAnywhere)
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere)
	UCameraComponent* ViewCamera;

	// played on retry (ResetForFlowRestart)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Flow")
	UAnimMontage* GetUpMontage = nullptr;
};
