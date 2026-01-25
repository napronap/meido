// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "ActorComponents/MeiDouComponent.h"
#include "GameFramework/Character.h"
#include "Types/CharacterTypes.h"
#include "Interfaces/ComboAttacker.h"
#include "MaidCharacter.generated.h"

class UAttackComponent;
class UMeiDouComponent;
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
	UPROPERTY(EditAnywhere, Category = "Input|Basic")
	UInputAction* MovementAction;

	UPROPERTY(EditAnywhere, Category = "Input|Basic")
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, Category = "Input|Basic")
	UInputAction* JumpAction;
	virtual void Jump() override;
	virtual void StopJumping() override;

	UPROPERTY(EditAnywhere, Category = "Input|Basic")
	UInputAction* ComboAttackAction;

	/* Combat */
	/* Basic combat */
	UPROPERTY(EditAnywhere, Category="Combat|Combo")
	UAnimMontage* ComboAttackMontage;
	
	// names of the sections in the anim montage defined above
	UPROPERTY(EditAnywhere, Category="Combat|Combo")
	TArray<FName> ComboSectionNames;

	int32 ComboIndex = 0;
	float CachedAttackInputTime = 0.0f;
	int32 ComboCount = 0;

	// notifies
	virtual void CheckCombo_Implementation() override;
	virtual void RecoveryEnd_Implementation() override;
	
	FOnMontageEnded OnAttackMontageEnded;
	void AttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UAttackComponent* AttackComponent;

	/* mei dou */
	UPROPERTY()
	UMeiDouComponent* MeiDouComponent;

	UPROPERTY(EditDefaultsOnly, Category="Input|MeiDou")
	UInputAction* MeiDouMAction;

	UPROPERTY(EditDefaultsOnly, Category="Input|MeiDou")
	UInputAction* MeiDouKAction;
	
	UPROPERTY(EditDefaultsOnly, Category="Input|MeiDou")
	UInputAction* MeiDouNAction;

	// needs to be on the system because it will be attached to a delegate
	// apparently this is only a thing for dynamic delegates
	UFUNCTION()
	void HandleMeiDouComboResolved(const FMeiDouResolvedCombo& Result);

	/* Action callbacks */
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void ComboAttackStart();
	void ComboAttack();
	void RegisterMeiDouInput(const EMeiDouInput Input);
	void InputMeiDouM();
	void InputMeiDouK();
	void InputMeiDouN();
	
private:
	ECharacterState CharacterState = ECharacterState::ECS_Idle;

	UPROPERTY(VisibleAnywhere)
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere)
	UCameraComponent* ViewCamera;
};
