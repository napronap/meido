// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/MaidCharacter.h"
#include "Components/InputComponent.h"
#include "ActorComponents/AttackComponent.h"
#include "ActorComponents/LockOnComponent.h"
#include "ActorComponents/MeiDouComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimMontage.h"

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
}

void AMaidCharacter::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr)
	{
		const FRotator ControlRotation = GetControlRotation();

		// yaw (horizontal) rotation, create a rotator that represents the controller's rotation
		// only using yaw because we don't want the character to lay down or else
		const FRotator YawRotation(0.f, ControlRotation.Yaw, 0.f);

		// given the yaw rotation, give me the X unit in the matrix (forward)
		// X is forward in UE!!
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// same thing, given the yaw rotation give me the Y, since Y is right in UE
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// scale that forward by mov vector Y because we have Y axis representing fw/bw directions
		AddMovementInput(ForwardDirection, Forward);
		// and scale by our input X, which represents X (horizontal) movement
		AddMovementInput(RightDirection, Right);
	}
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

void AMaidCharacter::DoLook(float Yaw, float Pitch)
{
	AddControllerPitchInput(Pitch);
	AddControllerYawInput(Yaw);
}

void AMaidCharacter::DoStartComboAttack()
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

		DoContinueCombo();
	}
}

void AMaidCharacter::DoContinueCombo()
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

void AMaidCharacter::RegisterMeiDouInput(const EMeiDouInput Input)
{
	if (!MeiDouComponent) return;

	if (UMeiDouPoseDataAsset* Pose = MeiDouPoseDataMap[Input])
	{
		MeiDouComponent->RegisterInput(Input);

		const int32 InputCount = MeiDouComponent->GetInputCount(Input);
		
		PlayAnimMontage(
			InputCount % 2 == 0 ? Pose->MirroredMontage : Pose->Montage,
			Pose->PlayRate,
			FName("Pose")
		);
	}
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
