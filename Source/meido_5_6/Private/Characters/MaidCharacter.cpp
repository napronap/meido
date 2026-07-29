// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/MaidCharacter.h"
#include "Components/InputComponent.h"
#include "ActorComponents/AttackComponent.h"
#include "ActorComponents/CharacterStateComponent.h"
#include "ActorComponents/DashComponent.h"
#include "ActorComponents/HealthComponent.h"
#include "ActorComponents/LockOnComponent.h"
#include "ActorComponents/MeiDouComponent.h"
#include "AnimInstances/MaidAnimInstance.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimMontage.h"
#include "IA/EnemyMaidAIController.h"

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
	OnDamageMontageEnded.BindUObject(this, &AMaidCharacter::DamageMontageEnded);
	OnMeiDouFailMontageEnded.BindUObject(this, &AMaidCharacter::MeiDouFailMontageEnded);

	// CP0.1: layered state scaffold (gates migrate in CP0.2)
	CharacterStateComponent = CreateDefaultSubobject<UCharacterStateComponent>(TEXT("CharacterStateComponent"));
}

// Called when the game starts or when spawned
void AMaidCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (!CharacterStateComponent)
	{
		CharacterStateComponent = FindComponentByClass<UCharacterStateComponent>();
	}

	AttackComponent = FindComponentByClass<UAttackComponent>();
	DashComponent = FindComponentByClass<UDashComponent>();
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

		MeiDouComponent->OnComboFailed.AddDynamic(
			this,
			&AMaidCharacter::HandleMeiDouComboFailed
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

void AMaidCharacter::ApplyGameplayStateIdle()
{
	if (!CharacterStateComponent)
	{
		return;
	}

	CharacterStateComponent->SetAttackState(EAttackState::None);
	CharacterStateComponent->SetMeiDouLayerState(EMeiDouLayerState::Idle);
	if (CharacterStateComponent->GetHealthState() == EHealthActionState::Stagger)
	{
		CharacterStateComponent->SetHealthState(EHealthActionState::Alive);
	}
	if (CharacterStateComponent->GetLocomotionState() == ELocomotionState::Jump
		|| CharacterStateComponent->GetLocomotionState() == ELocomotionState::Dash)
	{
		CharacterStateComponent->SetLocomotionState(ELocomotionState::Grounded);
	}
}

void AMaidCharacter::ApplyGameplayStateAttacking()
{
	if (!CharacterStateComponent)
	{
		return;
	}

	CharacterStateComponent->SetAttackState(EAttackState::InSwing);
}

void AMaidCharacter::ApplyGameplayStateWhiffRecover()
{
	if (!CharacterStateComponent)
	{
		return;
	}

	CharacterStateComponent->SetAttackState(EAttackState::WhiffRecover);
}

void AMaidCharacter::ApplyGameplayStateStagger()
{
	bDamageReactionActive = true;
	if (!CharacterStateComponent)
	{
		return;
	}

	CharacterStateComponent->SetAttackState(EAttackState::None);
	CharacterStateComponent->SetMeiDouLayerState(EMeiDouLayerState::Idle);
	CharacterStateComponent->SetLocomotionState(ELocomotionState::Grounded);
	CharacterStateComponent->SetHealthState(EHealthActionState::Stagger);
}

void AMaidCharacter::ApplyGameplayStateJumping()
{
	if (!CharacterStateComponent)
	{
		return;
	}

	CharacterStateComponent->SetLocomotionState(ELocomotionState::Jump);
}

void AMaidCharacter::ApplyGameplayStateDashing()
{
	if (!CharacterStateComponent)
	{
		return;
	}

	CharacterStateComponent->SetAttackState(EAttackState::None);
	CharacterStateComponent->SetLocomotionState(ELocomotionState::Dash);
}

void AMaidCharacter::ApplyGameplayStateMeiDouActive()
{
	if (!CharacterStateComponent)
	{
		return;
	}

	CharacterStateComponent->SetAttackState(EAttackState::None);
	CharacterStateComponent->SetMeiDouLayerState(EMeiDouLayerState::Active);
}

void AMaidCharacter::ApplyGameplayStateMeiDouFailed()
{
	if (!CharacterStateComponent)
	{
		return;
	}

	CharacterStateComponent->SetAttackState(EAttackState::None);
	CharacterStateComponent->SetMeiDouLayerState(EMeiDouLayerState::Failed);
}

void AMaidCharacter::ApplyGameplayStateDead()
{
	bDamageReactionActive = false;
	if (!CharacterStateComponent)
	{
		return;
	}

	CharacterStateComponent->SetAttackState(EAttackState::None);
	CharacterStateComponent->SetMeiDouLayerState(EMeiDouLayerState::Idle);
	CharacterStateComponent->SetLocomotionState(ELocomotionState::Grounded);
	CharacterStateComponent->SetHealthState(EHealthActionState::Dead);
}

float AMaidCharacter::TakeDamage(
	float DamageAmount,
	FDamageEvent const& DamageEvent,
	AController* EventInstigator,
	AActor* DamageCauser
)
{
	if (bHasDied || DamageAmount <= 0.f)
	{
		return 0.f;
	}

	if (CharacterStateComponent
		&& CharacterStateComponent->GetMeiDouLayerState() == EMeiDouLayerState::Failed)
	{
		return 0.f;
	}

	if (HasActiveIFrames())
	{
		return 0.f;
	}

	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

void AMaidCharacter::DoMove(float Right, float Forward)
{
	if (CharacterStateComponent
		&& (CharacterStateComponent->IsMeiDouLocked() || CharacterStateComponent->IsDashing()))
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
	if (!CharacterStateComponent)
	{
		return;
	}

	if (CharacterStateComponent->GetMeiDouLayerState() == EMeiDouLayerState::Failed)
	{
		return;
	}

	// only jump in idle, so we don't have weird behavior of jumping while character is playing the attack montage
	if (CharacterStateComponent->GetOverall() == ECharacterOverallState::Idle)
	{
		ApplyGameplayStateJumping();
		Super::Jump();
	}
}

void AMaidCharacter::StopJumping()
{
	if (CharacterStateComponent
		&& CharacterStateComponent->GetLocomotionState() == ELocomotionState::Jump)
	{
		ApplyGameplayStateIdle();
	}

	Super::StopJumping();
}

void AMaidCharacter::DoLook(float Yaw, float Pitch)
{
	AddControllerPitchInput(Pitch);
	AddControllerYawInput(Yaw);
}

bool AMaidCharacter::DoDash(const FVector2D& MoveInput, bool bLockOnActive)
{
	if (!DashComponent)
	{
		return false;
	}

	if (CharacterStateComponent
		&& CharacterStateComponent->GetMeiDouLayerState() == EMeiDouLayerState::Failed)
	{
		return false;
	}

	const FRotator ControlOrActorRotation = Controller
		? Controller->GetControlRotation()
		: GetActorRotation();

	return DashComponent->TryDash(MoveInput, ControlOrActorRotation, bLockOnActive);
}

void AMaidCharacter::DoStartComboAttack()
{
	if (!CharacterStateComponent)
	{
		return;
	}

	if (CharacterStateComponent->IsMeiDouLocked()
		|| CharacterStateComponent->IsDashing()
		|| CharacterStateComponent->IsStaggered()
		|| CharacterStateComponent->GetAttackState() == EAttackState::WhiffRecover
		|| bDamageReactionActive)
	{
		return;
	}

	if (DamageMontage)
	{
		if (UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
		{
			if (AnimInstance->Montage_IsPlaying(DamageMontage))
			{
				return;
			}
		}
	}

	// only attack if we are not in the air, at least for now
	if (!GetCharacterMovement()->IsFalling())
	{
		// if we are currently attacking, register this attack
		const bool bAttacking = CharacterStateComponent->IsAttacking()
			&& CharacterStateComponent->GetAttackState() != EAttackState::WhiffRecover;
		if (bAttacking)
		{
			CachedAttackInputTime = GetWorld()->GetTimeSeconds();

			return;
		}

		DoContinueCombo();
	}
}

void AMaidCharacter::DoContinueCombo()
{
	if (!AttackComponent || !ComboAttackMontage)
	{
		ApplyGameplayStateIdle();
		return;
	}

	AttackComponent->StartAttack();
	ApplyGameplayStateAttacking();

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
		else
		{
			ApplyGameplayStateIdle();
		}
	}
	else
	{
		ApplyGameplayStateIdle();
	}
}

void AMaidCharacter::CheckCombo_Implementation()
{
	const bool bAttacking = CharacterStateComponent
		&& CharacterStateComponent->IsAttacking()
		&& CharacterStateComponent->GetAttackState() != EAttackState::WhiffRecover;

	if (bAttacking)
	{
		// if there were no inputs since our last input, don't continue the combo and enter recovery mode
		if (CachedAttackInputTime <= 0.f)
		{
			ApplyGameplayStateWhiffRecover();
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
	if (bHasDied)
	{
		return;
	}

	ComboCount = 0;
	CachedAttackInputTime = 0.f;

	if (CharacterStateComponent
		&& CharacterStateComponent->GetMeiDouLayerState() == EMeiDouLayerState::Active)
	{
		return;
	}

	if (UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
	{
		if (DamageMontage && AnimInstance->Montage_IsPlaying(DamageMontage))
		{
			ApplyGameplayStateStagger();
			return;
		}
	}

	if (bDamageReactionActive)
	{
		ApplyGameplayStateStagger();
		return;
	}

	ApplyGameplayStateIdle();
}

void AMaidCharacter::DamageMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	bDamageReactionActive = false;

	if (bHasDied)
	{
		return;
	}

	if (HealthComponent && HealthComponent->IsDead())
	{
		return;
	}

	const bool bMeiDouActive = CharacterStateComponent
		&& CharacterStateComponent->GetMeiDouLayerState() == EMeiDouLayerState::Active;
	if (!bMeiDouActive)
	{
		ApplyGameplayStateIdle();
	}
}

void AMaidCharacter::MeiDouFailMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (bHasDied)
	{
		return;
	}

	if (CharacterStateComponent
		&& CharacterStateComponent->GetMeiDouLayerState() == EMeiDouLayerState::Failed)
	{
		ApplyGameplayStateIdle();
	}
}

void AMaidCharacter::HandleDamageTaken(
	UHealthComponent* InHealthComponent,
	float Damage,
	float CurrentHealth,
	AActor* DamageCauser,
	AController* InstigatedBy
)
{
	// MeiDou (pose/result) has super armor: take damage but do not interrupt action montages
	// should probably introduce a poise mechanic in the future...
	if (MeiDouComponent && MeiDouComponent->GetMeiDouState() != EMeiDouState::EMDS_Idle)
	{
		return;
	}

	if (!DamageMontage)
	{
		if (IsPlayerControlled())
		{
			GrantIFrames(PostDamageIFrameDuration);
		}
		return;
	}

	if (AttackComponent)
	{
		AttackComponent->CloseHitWindow();
	}

	if (DashComponent)
	{
		DashComponent->CancelDash();
	}

	UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	if (!AnimInstance)
	{
		ApplyGameplayStateIdle();
		return;
	}

	// Damage reactions should interrupt any ongoing action montage.
	AnimInstance->Montage_Stop(0.05f);

	ApplyGameplayStateStagger();

	if (IsPlayerControlled())
	{
		GrantIFrames(PostDamageIFrameDuration);
	}

	FName SectionToPlay = NAME_None;
	if (DamageSectionNames.Num() > 0)
	{
		const int32 SafeIndex = FMath::Abs(NextDamageSectionIndex) % DamageSectionNames.Num();
		SectionToPlay = DamageSectionNames[SafeIndex];
		NextDamageSectionIndex = (SafeIndex + 1) % DamageSectionNames.Num();
	}

	const float MontageLength = PlayAnimMontage(DamageMontage, 1.f, SectionToPlay);
	if (MontageLength > 0.f)
	{
		AnimInstance->Montage_SetEndDelegate(OnDamageMontageEnded, DamageMontage);
		return;
	}

	// fallback if no section name
	// TODO: remove, configurations should always exist
	const float FallbackLength = PlayAnimMontage(DamageMontage, 1.f);
	if (FallbackLength > 0.f)
	{
		AnimInstance->Montage_SetEndDelegate(OnDamageMontageEnded, DamageMontage);
		return;
	}

	bDamageReactionActive = false;
	ApplyGameplayStateIdle();
}

void AMaidCharacter::HandleHealthDepleted(UHealthComponent* InHealthComponent, AActor* DamageCauser)
{
	if (bHasDied)
	{
		return;
	}
	bHasDied = true;

	if (AttackComponent)
	{
		AttackComponent->CloseHitWindow();
	}

	if (DashComponent)
	{
		DashComponent->CancelDash();
	}

	// stop controller driven yaw updates on dead bodies (dead maids were still rotating to character)
	bUseControllerRotationYaw = false;
	if (AEnemyMaidAIController* EnemyAIController = Cast<AEnemyMaidAIController>(GetController()))
	{
		EnemyAIController->SetAIEnabled(false);
	}

	ApplyGameplayStateDead();

	if (UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
	{
		AnimInstance->Montage_Stop(0.05f);
		if (DeathMontage)
		{
			PlayAnimMontage(DeathMontage);
		}
	}

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->StopMovementImmediately();
		MovementComponent->DisableMovement();
	}

	if (DeathLifeSpanSeconds > 0.f)
	{
		SetLifeSpan(DeathLifeSpanSeconds);
	}
}

void AMaidCharacter::RegisterMeiDouInput(const EMeiDouInput Input)
{
	if (!MeiDouComponent)
	{
		return;
	}

	const bool bMeiDouGateOk = CharacterStateComponent
		&& (CharacterStateComponent->GetOverall() == ECharacterOverallState::Idle
			|| CharacterStateComponent->GetMeiDouLayerState() == EMeiDouLayerState::Active);

	if (!bMeiDouGateOk || GetCharacterMovement()->IsFalling())
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
			// play result montages from their default start, unlike pose inputs that always use "Pose" section
			// TODO: make more consistent (either start all from "Pose" or not)
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

void AMaidCharacter::HandleMeiDouComboFailed(const FMeiDouResolvedCombo& Result)
{
	if (!MeiDouComponent)
	{
		return;
	}

	SetMeiDouMirrorFlag(GetMesh(), false);

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->StopMovementImmediately();
	}

	ApplyGameplayStateMeiDouFailed();

	if (!MeiDouFailMontage)
	{
		ApplyGameplayStateIdle();
		MeiDouComponent->OnRequestedAnimationFailed();
		return;
	}

	const float MontageLength = PlayAnimMontage(MeiDouFailMontage, 1.f);
	if (MontageLength <= 0.f)
	{
		ApplyGameplayStateIdle();
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("Could not play MeiDou fail montage. Check montage slot setup.")
		);

		MeiDouComponent->OnRequestedAnimationFailed();
		return;
	}

	if (UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
	{
		AnimInstance->Montage_SetEndDelegate(OnMeiDouFailMontageEnded, MeiDouFailMontage);
	}

	if (!GEngine)
	{
		return;
	}

	GEngine->AddOnScreenDebugMessage(
		-1,
		2.0f,
		FColor::Yellow,
		TEXT("MeiDou combo failed")
	);
}

void AMaidCharacter::HandleMeiDouControlLockChanged(bool bIsLocked)
{
	if (!bIsLocked)
	{
		SetMeiDouMirrorFlag(GetMesh(), false);

		if (CharacterStateComponent
			&& CharacterStateComponent->GetMeiDouLayerState() == EMeiDouLayerState::Active)
		{
			ApplyGameplayStateIdle();
		}
		return;
	}

	ApplyGameplayStateMeiDouActive();
	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->StopMovementImmediately();
	}
}

bool AMaidCharacter::CanStartDash() const
{
	if (bHasDied || !CharacterStateComponent)
	{
		return false;
	}

	return CharacterStateComponent->GetOverall() == ECharacterOverallState::Idle;
}

void AMaidCharacter::NotifyDashStarted()
{
	if (bHasDied)
	{
		return;
	}

	if (AttackComponent)
	{
		AttackComponent->CloseHitWindow();
	}

	ApplyGameplayStateDashing();

	if (IsPlayerControlled())
	{
		GrantIFrames(DashIFrameDuration);
	}
}

void AMaidCharacter::NotifyDashEnded()
{
	if (bHasDied)
	{
		return;
	}

	if (CharacterStateComponent && CharacterStateComponent->IsDashing())
	{
		ApplyGameplayStateIdle();
	}
}

bool AMaidCharacter::IsDead() const
{
	if (bHasDied)
	{
		return true;
	}

	return HealthComponent && HealthComponent->IsDead();
}

void AMaidCharacter::ResetForFlowRestart()
{
	bHasDied = false;
	bDamageReactionActive = false;
	InvulnerableUntilTime = 0.f;
	ComboCount = 0;
	CachedAttackInputTime = 0.f;
	NextDamageSectionIndex = 0;

	if (CharacterStateComponent)
	{
		CharacterStateComponent->ResetToDefaults();
	}

	SetLifeSpan(0.f);

	if (AttackComponent)
	{
		AttackComponent->CloseHitWindow();
	}

	if (DashComponent)
	{
		DashComponent->CancelDash();
	}

	if (UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
	{
		AnimInstance->Montage_Stop(0.f);
	}

	if (HealthComponent)
	{
		HealthComponent->ResetToFullHealth();
	}

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->SetMovementMode(MOVE_Walking);
		MovementComponent->StopMovementImmediately();
	}
}

void AMaidCharacter::GrantIFrames(const float DurationSeconds)
{
	if (DurationSeconds <= 0.f || !GetWorld())
	{
		return;
	}

	const float NewUntil = GetWorld()->GetTimeSeconds() + DurationSeconds;
	InvulnerableUntilTime = FMath::Max(InvulnerableUntilTime, NewUntil);
}

bool AMaidCharacter::HasActiveIFrames() const
{
	if (!GetWorld())
	{
		return false;
	}

	return GetWorld()->GetTimeSeconds() < InvulnerableUntilTime;
}
