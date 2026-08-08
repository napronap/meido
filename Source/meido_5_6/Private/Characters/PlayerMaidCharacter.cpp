// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/PlayerMaidCharacter.h"

#include "EnhancedInputComponent.h"
#include "ActorComponents/CharacterStateComponent.h"
#include "ActorComponents/DashComponent.h"
#include "ActorComponents/LockOnComponent.h"
#include "ActorComponents/MaidCameraManagerComponent.h"
#include "AnimInstances/MaidAnimInstance.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"

APlayerMaidCharacter::APlayerMaidCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;
	
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 400.f, 0.f);
	
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>("CameraBoom");
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 300.f;
	CameraBoom->bUsePawnControlRotation = true;
	
	ViewCamera = CreateDefaultSubobject<UCameraComponent>("ViewCamera");
	ViewCamera->SetupAttachment(CameraBoom);

	CameraManager = CreateDefaultSubobject<UMaidCameraManagerComponent>(TEXT("CameraManager"));

	// CP0.1 smoke: on-screen Overall / Attack / Health (disable on component if noisy)
	if (CharacterStateComponent)
	{
		CharacterStateComponent->bDrawDebugState = true;
	}
}

ECombatTeam APlayerMaidCharacter::GetCombatTeam_Implementation() const
{
	return ECombatTeam::Player;
}

void APlayerMaidCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (CameraManager)
	{
		CameraManager->Initialize(CameraBoom, ViewCamera);
		CameraManager->BindLockOn(LockOnComponent);
	}
}

void APlayerMaidCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	const bool bLockOnActive = LockOnComponent && LockOnComponent->IsLockedOn();
	UpdateAnimLockOnState(bLockOnActive);
	ApplyLockOnMovementMode(bLockOnActive);
	// Camera Free lag + LockOn yaw: UMaidCameraManagerComponent
}

void APlayerMaidCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();
	CachedMoveInput = MovementVector;

	DoMove(MovementVector.X, MovementVector.Y);
}

void APlayerMaidCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	const bool bLockOnActive = LockOnComponent && LockOnComponent->IsLockedOn();

	// While lockon is active we keep yaw camera control driven by target tracking
	// Player can still adjust pitch manually
	const float YawInput = bLockOnActive ? 0.f : LookAxisVector.X;
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
	if (!DashComponent)
	{
		return;
	}

	const bool bLockOnActive = LockOnComponent && LockOnComponent->IsLockedOn();
	const FRotator ControlRotation = Controller ? Controller->GetControlRotation() : GetControlRotation();
	DashComponent->TryDash(CachedMoveInput, ControlRotation, bLockOnActive);
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
	if (!LockOnComponent)
	{
		return;
	}

	if (LockOnComponent->IsLockedOn())
	{
		LockOnComponent->ClearLockOn();
		UpdateAnimLockOnState(false);
		ApplyLockOnMovementMode(false);
	}
	else
	{
		const bool bLocked = LockOnComponent->TryLockOn();
		UpdateAnimLockOnState(bLocked);
		ApplyLockOnMovementMode(bLocked);
	}
}

void APlayerMaidCharacter::OnLockOnSwitch(const FInputActionValue& Value)
{
	if (!LockOnComponent)
	{
		return;
	}

	LockOnComponent->HandleSwitchInput(Value.Get<float>());
}

void APlayerMaidCharacter::OnLockOnSwitchReleased(const FInputActionValue& Value)
{
	if (!LockOnComponent) return;
	LockOnComponent->HandleSwitchReleased();
}

void APlayerMaidCharacter::ApplyLockOnMovementMode(bool bLockOnActive)
{
	if (bWasLockOnActive == bLockOnActive)
	{
		return;
	}

	bWasLockOnActive = bLockOnActive;

	if (bLockOnActive)
	{
		bUseControllerRotationYaw = true;
		GetCharacterMovement()->bOrientRotationToMovement = false;
	}
	else
	{
		bUseControllerRotationYaw = false;
		GetCharacterMovement()->bOrientRotationToMovement = true;
	}
}

void APlayerMaidCharacter::UpdateAnimLockOnState(bool bLockOnActive)
{
	if (!GetMesh())
	{
		return;
	}

	if (UMaidAnimInstance* MaidAnimInstance = Cast<UMaidAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		MaidAnimInstance->bIsLockedOn = bLockOnActive;
	}
}

void APlayerMaidCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// movement related actions
		EnhancedInputComponent->BindAction(MovementAction, ETriggerEvent::Triggered, this, &APlayerMaidCharacter::Move);
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
		EnhancedInputComponent->BindAction(ChangeLockOnInputAction, ETriggerEvent::Completed, this,
		                                   &APlayerMaidCharacter::OnLockOnSwitchReleased);
	}
}