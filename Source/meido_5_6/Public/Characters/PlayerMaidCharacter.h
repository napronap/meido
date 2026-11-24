// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MaidCharacter.h"
#include "PlayerMaidCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class ULockOnComponent;

// facial expressions
UENUM()
enum class EMaidExpression : uint8
{
	// no morph targets
	Neutral,
	// main menu
	Wink,
	// win
	Happy,
};

USTRUCT()
struct FMaidExpression
{
	GENERATED_BODY()

	// morph target name -> weight
	UPROPERTY(EditAnywhere)
	TMap<FName, float> MorphTargets;
};

// the AMaidCharacter actions plus dash, MeiDou, lockon and a camera boom
UCLASS()
class MEIDO_5_6_API APlayerMaidCharacter : public AMaidCharacter
{
	GENERATED_BODY()

public:
	APlayerMaidCharacter();

	// this method is exclusive for use in the GM_Meido (demo)
	// TODO: remove at some point
	virtual void ResetForFlowRestart() override;

	// will always drop and set the new instantly because God forbid I lerp the expressions
	void SetExpression(EMaidExpression Expression);

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
	
	UPROPERTY(EditDefaultsOnly, Category="Expression")
	TMap<EMaidExpression, FMaidExpression> Expressions;

	virtual bool CanTakeDamage() const override;
};
