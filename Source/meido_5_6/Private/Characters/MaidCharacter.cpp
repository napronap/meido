// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/MaidCharacter.h"
#include "Components/InputComponent.h"
#include "ActorComponents/AttackComponent.h"
#include "ActorComponents/DashComponent.h"
#include "ActorComponents/HealthComponent.h"
#include "ActorComponents/MeiDouComponent.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimMontage.h"
#include "IA/EnemyMaidAIController.h"

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

	AttackComponent = CreateDefaultSubobject<UAttackComponent>(TEXT("AttackComponent"));
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));

	OnDamageMontageEnded.BindUObject(this, &AMaidCharacter::DamageMontageEnded);
}

// Called when the game starts or when spawned
void AMaidCharacter::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AMaidCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

float AMaidCharacter::TakeDamage(
	float DamageAmount,
	FDamageEvent const& DamageEvent,
	AController* EventInstigator,
	AActor* DamageCauser
)
{
	if (bHasDied)
	{
		return 0.f;
	}

	if (IsMeiDouFailed())
	{
		return 0.f;
	}

	if (HasActiveIFrames())
	{
		return 0.f;
	}

	// Super resolves the damage event (point/radial) into the final amount
	const float Damage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	const float AppliedDamage = HealthComponent->ApplyDamage(Damage, DamageCauser);

	if (HealthComponent->IsDead())
	{
		Die();
	}
	else
	{
		PlayHitReaction();
	}

	return AppliedDamage;
}

void AMaidCharacter::DoMove(float Right, float Forward)
{
	// attack montages also block it through root motion, but the gate keeps it explicit (and covers AI input)
	if (IsAttacking() || IsMeiDouActive() || IsDashing() || IsMeiDouFailed())
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
	// only jump when free, so we don't have weird behavior of jumping while character is playing an action montage
	if (CanAct())
	{
		Super::Jump();
	}
}

void AMaidCharacter::DoLook(float Yaw, float Pitch)
{
	AddControllerPitchInput(Pitch);
	AddControllerYawInput(Yaw);
}

bool AMaidCharacter::DoDash(const FVector2D& MoveInput, bool bLockOnActive)
{
	// enemies have no dash component
	if (!DashComponent || !CanAct())
	{
		return false;
	}

	const FRotator ControlOrActorRotation = Controller
		? Controller->GetControlRotation()
		: GetActorRotation();

	if (!DashComponent->TryDash(MoveInput, ControlOrActorRotation, bLockOnActive))
	{
		return false;
	}

	if (IsPlayerControlled())
	{
		GrantIFrames(DashIFrameDuration);
	}

	return true;
}

void AMaidCharacter::DoStartComboAttack()
{
	// attacking is allowed: AttackComponent queues the next step itself
	if (IsLocked() || IsDashing() || IsMeiDouActive())
	{
		return;
	}

	AttackComponent->RequestAttack();
}

void AMaidCharacter::DamageMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	bDamageReactionActive = false;
}

void AMaidCharacter::PlayHitReaction()
{
	// MeiDou (pose/result) has super armor: take damage but do not interrupt action montages
	// should probably introduce a poise mechanic in the future...
	if (IsMeiDouActive())
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

	UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	if (!AnimInstance)
	{
		return;
	}

	// Damage reactions should interrupt any ongoing action montage.
	AnimInstance->Montage_Stop(0.05f);

	bDamageReactionActive = true;

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
}

void AMaidCharacter::Die()
{
	bHasDied = true;

	if (AttackComponent)
	{
		AttackComponent->CloseHitWindow();
	}

	// stop controller driven yaw updates on dead bodies (dead maids were still rotating to character)
	bUseControllerRotationYaw = false;
	if (AEnemyMaidAIController* EnemyAIController = Cast<AEnemyMaidAIController>(GetController()))
	{
		EnemyAIController->SetAIEnabled(false);
	}

	bDamageReactionActive = false;

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

	// MeiDou active is allowed: the component queues the next pose itself
	if (IsLocked() || IsAttacking() || IsDashing())
	{
		return;
	}

	MeiDouComponent->RegisterInput(Input);
}

bool AMaidCharacter::IsAttacking() const
{
	return AttackComponent->IsAttacking();
}

bool AMaidCharacter::IsDashing() const
{
	// enemies have no dash component
	return DashComponent && DashComponent->IsDashing();
}

bool AMaidCharacter::IsMeiDouActive() const
{
	// enemies have no MeiDou component
	return MeiDouComponent && MeiDouComponent->IsActive();
}

bool AMaidCharacter::IsMeiDouFailed() const
{
	return MeiDouComponent && MeiDouComponent->IsFailed();
}

bool AMaidCharacter::CanAct() const
{
	return !IsLocked() && !IsAttacking() && !IsDashing() && !IsMeiDouActive();
}

bool AMaidCharacter::IsLocked() const
{
	return IsDead() || bDamageReactionActive || IsMeiDouFailed() || GetCharacterMovement()->IsFalling();
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
	NextDamageSectionIndex = 0;

	SetLifeSpan(0.f);

	if (AttackComponent)
	{
		AttackComponent->CloseHitWindow();
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
