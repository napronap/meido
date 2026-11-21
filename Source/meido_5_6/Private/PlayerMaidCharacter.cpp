// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/PlayerMaidCharacter.h"
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "ActorComponents/DashComponent.h"
#include "ActorComponents/LockOnComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimMontage.h"

APlayerMaidCharacter::APlayerMaidCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 400.f, 0.f);

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 300.0f;
	CameraBoom->bUsePawnControlRotation = true;

	ViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ViewCamera"));
	ViewCamera->SetupAttachment(CameraBoom);

	// player-only gameplay components (pointers for Dash/MeiDou live in AMaidCharacter for now)
	DashComponent = CreateDefaultSubobject<UDashComponent>(TEXT("DashComponent"));
	MeiDouComponent = CreateDefaultSubobject<UMeiDouComponent>(TEXT("MeiDouComponent"));
	LockOnComponent = CreateDefaultSubobject<ULockOnComponent>(TEXT("LockOnComponent"));

	// Player should remain in scene on death for lose flow/camera
	// (different from enemy maid that uses this to despawn)
	DeathLifeSpanSeconds = 0.f;
}

void APlayerMaidCharacter::ResetForFlowRestart()
{
	Super::ResetForFlowRestart();

	if (LockOnComponent->IsLockedOn())
	{
		LockOnComponent->ClearLockOn();
	}

	if (GetUpMontage)
	{
		PlayAnimMontage(GetUpMontage);
	}
}


void APlayerMaidCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();
	CachedMoveInput = MovementVector;

	DoMove(MovementVector.X, MovementVector.Y);
}

void APlayerMaidCharacter::MoveReleased()
{
	CachedMoveInput = FVector2D::ZeroVector;
}

void APlayerMaidCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	// While lockon is active we keep yaw camera control driven by target tracking
	// Player can still adjust pitch manually
	const float YawInput = LockOnComponent->IsLockedOn() ? 0.f : LookAxisVector.X;
	DoLook(YawInput, LookAxisVector.Y);
}

void APlayerMaidCharacter::JumpPressed()
{
	Jump();
}

void APlayerMaidCharacter::JumpReleased()
{
	StopJumping();
}

void APlayerMaidCharacter::DashPressed()
{
	DoDash(CachedMoveInput, LockOnComponent->IsLockedOn());
}

void APlayerMaidCharacter::ComboAttackPressed()
{
	DoStartComboAttack();
}

void APlayerMaidCharacter::InputMeiDouM()
{
	RegisterMeiDouInput(EMeiDouInput::EMDI_Moe);
}

void APlayerMaidCharacter::InputMeiDouK()
{
	RegisterMeiDouInput(EMeiDouInput::EMDI_Kyun);
}

void APlayerMaidCharacter::InputMeiDouN()
{
	RegisterMeiDouInput(EMeiDouInput::EMDI_Nyan);
}


void APlayerMaidCharacter::ToggleLockOn()
{
	if (LockOnComponent->IsLockedOn())
	{
		LockOnComponent->ClearLockOn();
		return;
	}

	LockOnComponent->TryLockOn();
}

void APlayerMaidCharacter::OnLockOnSwitch(const FInputActionValue& Value)
{
	// bound on Started so it fires once per key press or stick push until it's released
	LockOnComponent->SwitchTarget(Value.Get<float>() > 0.f ? 1 : -1);
}

void APlayerMaidCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// movement related actions
		EnhancedInputComponent->BindAction(MovementAction, ETriggerEvent::Triggered, this, &APlayerMaidCharacter::Move);
		EnhancedInputComponent->BindAction(MovementAction, ETriggerEvent::Completed, this, &APlayerMaidCharacter::MoveReleased);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerMaidCharacter::Look);

		// combat related actions
		EnhancedInputComponent->BindAction(ComboAttackAction, ETriggerEvent::Started, this,
		                                   &APlayerMaidCharacter::ComboAttackPressed);

		// override inherited jump actions
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &APlayerMaidCharacter::JumpPressed);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &APlayerMaidCharacter::JumpReleased);
		EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Started, this, &APlayerMaidCharacter::DashPressed);

		// mei dou related actions
		EnhancedInputComponent->BindAction(MeiDouMAction, ETriggerEvent::Started, this, &APlayerMaidCharacter::InputMeiDouM);
		EnhancedInputComponent->BindAction(MeiDouKAction, ETriggerEvent::Started, this, &APlayerMaidCharacter::InputMeiDouK);
		EnhancedInputComponent->BindAction(MeiDouNAction, ETriggerEvent::Started, this, &APlayerMaidCharacter::InputMeiDouN);

		// lock on actions
		EnhancedInputComponent->BindAction(LockOnInputAction, ETriggerEvent::Started, this,
		                                   &APlayerMaidCharacter::ToggleLockOn);
		EnhancedInputComponent->BindAction(ChangeLockOnInputAction, ETriggerEvent::Started, this,
		                                   &APlayerMaidCharacter::OnLockOnSwitch);
	}
}
