// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "GameFramework/Character.h"
#include "Types/CharacterTypes.h"
#include "Interfaces/ComboAttacker.h"
#include "MaidCharacter.generated.h"

class UCombatComponent;
class UInputMappingContext;
class UInputAction;
class USpringArmComponent;
class UCameraComponent;
class UAnimMontage;

UCLASS()
class MEIDO_5_6_API AMaidCharacter : public ACharacter, public IComboAttacker
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMaidCharacter();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	virtual void BeginPlay() override;

	/* Input actions */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* MovementAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* ComboAttackAction;

	/* Combat */
	UPROPERTY(EditAnywhere, Category="Combat|Combo")
	UAnimMontage* ComboAttackMontage;

	// names of the sections in the anim montage defined above
	UPROPERTY(EditAnywhere, Category="Combat|Combo")
	TArray<FName> ComboSectionNames;

	int32 ComboIndex = 0;
	float CachedAttackInputTime = 0.0f;
	int32 ComboCount = 0;
	virtual void CheckCombo_Implementation() override;
	FOnMontageEnded OnAttackMontageEnded;
	void AttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	/* Action callbacks */
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void ComboAttackStart();
	void ComboAttack();
	
private:
	ECharacterState CharacterState = ECharacterState::ECS_Idle;

	UPROPERTY(VisibleAnywhere)
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere)
	UCameraComponent* ViewCamera;
};
