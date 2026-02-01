// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/MaidCharacter.h"
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "ActorComponents/AttackComponent.h"
#include "ActorComponents/LockOnComponent.h"
#include "ActorComponents/MeiDouComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimMontage.h"
#include "Interfaces/Targetable.h"

// Sets default values
AMaidCharacter::AMaidCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
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

	OnAttackMontageEnded.BindUObject(this, &AMaidCharacter::AttackMontageEnded);
}

// Called when the game starts or when spawned
void AMaidCharacter::BeginPlay()
{
	Super::BeginPlay();

	AttackComponent = FindComponentByClass<UAttackComponent>();
	MeiDouComponent = FindComponentByClass<UMeiDouComponent>();
	LockOnComponent = FindComponentByClass<ULockOnComponent>();

	if (MeiDouComponent)
	{
		MeiDouComponent->OnComboResolved.AddDynamic(
			this,
			&AMaidCharacter::HandleMeiDouComboResolved
		);
	}
}

// Called every frame
void AMaidCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (LockOnComponent && LockOnComponent->IsLockedOn())
	{
		AActor* Target = LockOnComponent->GetCurrentTarget();
		if (Target)
		{
			RotateCameraToTarget(Target, DeltaTime);
		}
	}

	const UEnum* EnumPtr = StaticEnum<ECharacterState>();
	FString Message = FString::Printf(
		TEXT("Character State: %s"), *EnumPtr->GetNameStringByValue(static_cast<int64>(CharacterState)));
	GEngine->AddOnScreenDebugMessage(1, 0.f, FColor::Red, Message);
}

void AMaidCharacter::RotateCameraToTarget(AActor* Target, float DeltaTime)
{
	FVector TargetLocation;

	if (Target->Implements<UTargetable>())
	{
		TargetLocation =
			ITargetable::Execute_GetTargetLocation(Target);
	}
	else
	{
		TargetLocation = Target->GetActorLocation();
	}

	const FVector CameraLocation = ViewCamera->GetComponentLocation();
	const FVector Direction = (TargetLocation - CameraLocation).GetSafeNormal();

	FRotator DesiredRotation = Direction.Rotation();

	FRotator CurrentRotation = Controller->GetControlRotation();
	FRotator NewRotation(
		CurrentRotation.Pitch,
		DesiredRotation.Yaw,
		CurrentRotation.Roll
	);

	Controller->SetControlRotation(NewRotation);
}

void AMaidCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();

	const FRotator ControlRotation = GetControlRotation();

	// yaw (horizontal) rotation, create a rotator that represents the controller's rotation
	// only using yaw because we don't want the character to lay down or else
	const FRotator YawRotation(0.f, ControlRotation.Yaw, 0.f);

	// given the yaw rotation, give me the X unit in the matrix (forward)
	// X is forward in UE!!
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	// scale that forward by mov vector Y because we have Y axis representing fw/bw directions
	AddMovementInput(ForwardDirection, MovementVector.Y);

	// same thing, given the yaw rotation give me the Y, since Y is right in UE
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	// and scale by our input X, which represents X (horizontal) movement
	AddMovementInput(RightDirection, MovementVector.X);
}

void AMaidCharacter::Jump()
{
	// only jump in idle, so we don't have weird behavior of jumping while character is playing the attack montage
	if (CharacterState == ECharacterState::ECS_Idle)
	{
		CharacterState = ECharacterState::ECS_Jumping;
		Super::Jump();
	}
}

void AMaidCharacter::StopJumping()
{
	CharacterState = ECharacterState::ECS_Idle;
	Super::StopJumping();
}

void AMaidCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	AddControllerPitchInput(LookAxisVector.Y);
	AddControllerYawInput(LookAxisVector.X);
}

void AMaidCharacter::ComboAttackStart()
{
	// only attack if we are not in the air, at least for now
	if (!GetCharacterMovement()->IsFalling())
	{
		// if we are currently attacking, register this attack
		if (CharacterState == ECharacterState::ECS_Attacking)
		{
			CachedAttackInputTime = GetWorld()->GetTimeSeconds();

			return;
		}

		ComboAttack();
	}
}

void AMaidCharacter::ComboAttack()
{
	AttackComponent->StartAttack();
	CharacterState = ECharacterState::ECS_Attacking;

	ComboCount = 0;

	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		const float MontageLength = AnimInstance->Montage_Play(
			ComboAttackMontage, 1.0f, EMontagePlayReturnType::MontageLength, 0.0f, true
		);

		if (MontageLength > 0.0f)
		{
			AnimInstance->Montage_SetEndDelegate(OnAttackMontageEnded, ComboAttackMontage);
		}
	}
}

void AMaidCharacter::CheckCombo_Implementation()
{
	if (CharacterState == ECharacterState::ECS_Attacking)
	{
		// if there were no inputs since our last input, don't continue the combo and enter recovery mode
		if (CachedAttackInputTime <= 0.f)
		{
			CharacterState = ECharacterState::ECS_Recovering;
			return;
		}

		// reset the count so the player needs to make another input to continue the chain
		CachedAttackInputTime = 0.f;

		++ComboCount;

		if (ComboCount < ComboSectionNames.Num())
		{
			if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
			{
				AnimInstance->Montage_JumpToSection(ComboSectionNames[ComboCount], ComboAttackMontage);
			}
		}
	}
}

void AMaidCharacter::RecoveryEnd_Implementation()
{
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		AnimInstance->Montage_Stop(.3f, ComboAttackMontage);
	}
}

void AMaidCharacter::AttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	CharacterState = ECharacterState::ECS_Idle;
	ComboCount = 0;
	CachedAttackInputTime = 0.f;
}

void AMaidCharacter::InputMeiDouM()
{
	RegisterMeiDouInput(EMeiDouInput::EMDI_Moe);
}

void AMaidCharacter::InputMeiDouK()
{
	RegisterMeiDouInput(EMeiDouInput::EMDI_Kyun);
}

void AMaidCharacter::InputMeiDouN()
{
	RegisterMeiDouInput(EMeiDouInput::EMDI_Nyan);
}

void AMaidCharacter::ToggleLockOn()
{
	if (!LockOnComponent) return;


	if (LockOnComponent->IsLockedOn())
	{
		LockOnComponent->ClearLockOn();
		GEngine->AddOnScreenDebugMessage(
			-1, 1.5f, FColor::Yellow, TEXT("Lock on cleared")
		);
	}
	else
	{
		const bool bLocked = LockOnComponent->TryLockOn();

		if (bLocked && GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1, 1.5f, FColor::Yellow, TEXT("LOCK ON")
			);
		}
	}
}

void AMaidCharacter::OnLockOnSwitch(const FInputActionValue& Value)
{
	if (!LockOnComponent) return;
	const float Axis = Value.Get<float>();

	const FString Message = FString::Printf(
		TEXT("Lockon switch input: %f"),
		Axis
	);

	GEngine->AddOnScreenDebugMessage(
		-1, 1.5f, FColor::Yellow, Message
	);

	LockOnComponent->HandleSwitchInput(Axis);
}


void AMaidCharacter::RegisterMeiDouInput(const EMeiDouInput Input)
{
	if (!MeiDouComponent) return;
	MeiDouComponent->RegisterInput(Input);
}

void AMaidCharacter::HandleMeiDouComboResolved(const FMeiDouResolvedCombo& Result)
{
	if (!GEngine)
	{
		return;
	}

	const FString Message = FString::Printf(
		TEXT("MeiDou Combo Resolved: %s"),
		*Result.ComboId.ToString()
	);

	GEngine->AddOnScreenDebugMessage(
		-1,
		2.5f,
		FColor::Green,
		Message
	);
}


// Called to bind functionality to input
void AMaidCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// movement related actions
		EnhancedInputComponent->BindAction(MovementAction, ETriggerEvent::Triggered, this, &AMaidCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMaidCharacter::Look);

		// combat related actions
		EnhancedInputComponent->BindAction(ComboAttackAction, ETriggerEvent::Started, this,
		                                   &AMaidCharacter::ComboAttackStart);

		// override inherited jump actions
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AMaidCharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AMaidCharacter::StopJumping);

		// mei dou related actions
		EnhancedInputComponent->BindAction(MeiDouMAction, ETriggerEvent::Started, this, &AMaidCharacter::InputMeiDouM);
		EnhancedInputComponent->BindAction(MeiDouKAction, ETriggerEvent::Started, this, &AMaidCharacter::InputMeiDouK);
		EnhancedInputComponent->BindAction(MeiDouNAction, ETriggerEvent::Started, this, &AMaidCharacter::InputMeiDouN);

		// lock on actions
		EnhancedInputComponent->BindAction(LockOnInputAction, ETriggerEvent::Started, this,
		                                   &AMaidCharacter::ToggleLockOn);
		EnhancedInputComponent->BindAction(ChangeLockOnInputAction, ETriggerEvent::Started, this,
		                                   &AMaidCharacter::OnLockOnSwitch);
	}
}
