// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/MaidCharacter.h"
#include "Components/InputComponent.h"
#include "ActorComponents/AttackComponent.h"
#include "ActorComponents/HealthComponent.h"
#include "ActorComponents/LockOnComponent.h"
#include "ActorComponents/MeiDouComponent.h"
#include "AnimInstances/MaidAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimMontage.h"

namespace
{
const FName MeiDouPoseSectionName(TEXT("Pose"));

void SetMeiDouMirrorFlag(USkeletalMeshComponent* Mesh, bool bShouldMirror)
{
	if (!Mesh)
	{
		return;
	}

	if (UMaidAnimInstance* MaidAnimInstance = Cast<UMaidAnimInstance>(Mesh->GetAnimInstance()))
	{
		MaidAnimInstance->bShouldMirror = bShouldMirror;
	}
}
}

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
	HealthComponent = FindComponentByClass<UHealthComponent>();
	MeiDouComponent = FindComponentByClass<UMeiDouComponent>();
	LockOnComponent = FindComponentByClass<ULockOnComponent>();

	if (HealthComponent)
	{
		HealthComponent->OnDamageTaken.AddDynamic(this, &AMaidCharacter::HandleDamageTaken);
		HealthComponent->OnHealthDepleted.AddDynamic(this, &AMaidCharacter::HandleHealthDepleted);
	}

	if (MeiDouComponent)
	{
		MeiDouComponent->OnPoseAnimationRequested.AddDynamic(
			this,
			&AMaidCharacter::HandleMeiDouPoseAnimationRequested
		);

		MeiDouComponent->OnComboResolved.AddDynamic(
			this,
			&AMaidCharacter::HandleMeiDouComboResolved
		);

		MeiDouComponent->OnMeiDouControlLockChanged.AddDynamic(
			this,
			&AMaidCharacter::HandleMeiDouControlLockChanged
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
	if (CharacterState == ECharacterState::ECS_MeiDouActive)
	{
		return;
	}

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
	if (CharacterState == ECharacterState::ECS_Jumping)
	{
		CharacterState = ECharacterState::ECS_Idle;
	}

	Super::StopJumping();
}

void AMaidCharacter::DoLook(float Yaw, float Pitch)
{
	AddControllerPitchInput(Pitch);
	AddControllerYawInput(Yaw);
}

void AMaidCharacter::DoStartComboAttack()
{
	if (CharacterState == ECharacterState::ECS_MeiDouActive || CharacterState == ECharacterState::ECS_Recovering)
	{
		return;
	}

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
	if (CharacterState != ECharacterState::ECS_MeiDouActive)
	{
		CharacterState = ECharacterState::ECS_Idle;
	}

	ComboCount = 0;
	CachedAttackInputTime = 0.f;
}

void AMaidCharacter::HandleDamageTaken(
	UHealthComponent* InHealthComponent,
	float Damage,
	float CurrentHealth,
	AActor* DamageCauser,
	AController* InstigatedBy
)
{
	if (!DamageMontage)
	{
		return;
	}

	if (DamageSectionNames.Num() > 0)
	{
		const int32 SafeIndex = FMath::Abs(NextDamageSectionIndex) % DamageSectionNames.Num();
		const FName SectionToPlay = DamageSectionNames[SafeIndex];

		const float MontageLength = PlayAnimMontage(DamageMontage, 1.f, SectionToPlay);
		if (MontageLength <= 0.f)
		{
			// If a configured section is missing, still play the montage from start.
			PlayAnimMontage(DamageMontage);
		}

		NextDamageSectionIndex = (SafeIndex + 1) % DamageSectionNames.Num();
		return;
	}

	PlayAnimMontage(DamageMontage);
}

void AMaidCharacter::HandleHealthDepleted(UHealthComponent* InHealthComponent, AActor* DamageCauser)
{
	CharacterState = ECharacterState::ECS_Recovering;

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->StopMovementImmediately();
		MovementComponent->DisableMovement();
	}
}

void AMaidCharacter::RegisterMeiDouInput(const EMeiDouInput Input)
{
	if (!MeiDouComponent)
	{
		return;
	}

	if (CharacterState != ECharacterState::ECS_Idle &&
		CharacterState != ECharacterState::ECS_MeiDouActive)
	{
		return;
	}

	MeiDouComponent->RegisterInput(Input);
}

void AMaidCharacter::HandleMeiDouPoseAnimationRequested(const FMeiDouPoseAnimationRequest& Request)
{
	if (!MeiDouComponent)
	{
		return;
	}

	const UMeiDouPoseDataAsset* Pose = MeiDouPoseDataMap.FindRef(Request.Input);
	if (!Pose)
	{
		MeiDouComponent->OnRequestedAnimationFailed();
		return;
	}

	SetMeiDouMirrorFlag(GetMesh(), Request.bUseMirroredMontage);

	UAnimMontage* PoseMontage = Pose->Montage;
	if (!PoseMontage)
	{
		MeiDouComponent->OnRequestedAnimationFailed();
		return;
	}

	const float MontageLength = PlayAnimMontage(PoseMontage, Pose->PlayRate, MeiDouPoseSectionName);
	if (MontageLength <= 0.f)
	{
		MeiDouComponent->OnRequestedAnimationFailed();
	}
}

void AMaidCharacter::HandleMeiDouComboResolved(const FMeiDouResolvedCombo& Result)
{
	if (!MeiDouComponent)
	{
		return;
	}

	SetMeiDouMirrorFlag(GetMesh(), false);

	if (const FMeiDouComboDefinition* ComboDefinition = MeiDouComponent->GetActiveComboDefinition())
	{
		if (ComboDefinition->AnimationMontage)
		{
			// Result montages are played from their default start, unlike pose inputs that always use "Pose" section.
			const float MontageLength = PlayAnimMontage(ComboDefinition->AnimationMontage, 1.f);
			if (MontageLength <= 0.f)
			{
				UE_LOG(
					LogTemp,
					Warning,
					TEXT("Could not play MeiDou result montage for combo '%s'. Check montage slot setup."),
					*ComboDefinition->ComboId.ToString()
				);
				if (GEngine)
				{
					const FString FailMessage = FString::Printf(
						TEXT("MeiDou result montage failed: %s"),
						*GetNameSafe(ComboDefinition->AnimationMontage)
					);
					GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, FailMessage);
				}
				MeiDouComponent->OnRequestedAnimationFailed();
			}
		}
		else
		{
			MeiDouComponent->OnRequestedAnimationFailed();
		}
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				2.0f,
				FColor::Red,
				TEXT("MeiDou Combo Resolved but no active definition")
			);
		}
		MeiDouComponent->OnRequestedAnimationFailed();
	}

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

void AMaidCharacter::HandleMeiDouControlLockChanged(bool bIsLocked)
{
	if (!bIsLocked)
	{
		SetMeiDouMirrorFlag(GetMesh(), false);

		if (CharacterState == ECharacterState::ECS_MeiDouActive)
		{
			CharacterState = ECharacterState::ECS_Idle;
		}
		return;
	}

	CharacterState = ECharacterState::ECS_MeiDouActive;
	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->StopMovementImmediately();
	}
}
