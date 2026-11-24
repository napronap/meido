// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/PlayerMaidCharacter.h"
#include "ActorComponents/DashComponent.h"
#include "ActorComponents/LockOnComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimMontage.h"

APlayerMaidCharacter::APlayerMaidCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 600.f, 0.f);

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 300.0f;
	CameraBoom->bUsePawnControlRotation = true;
	// spring arm's rotation NEEDS to be absolute so it doesn't conflict with the camera manager and manual rotations
	// that dash and attack component do
	CameraBoom->SetUsingAbsoluteRotation(true);

	ViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ViewCamera"));
	ViewCamera->SetupAttachment(CameraBoom);

	// player only gameplay components (pointers for Dash/MeiDou live in AMaidCharacter for now)
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

void APlayerMaidCharacter::SetExpression(const EMaidExpression Expression)
{
	GetMesh()->ClearMorphTargets();

	if (const FMaidExpression* Found = Expressions.Find(Expression))
	{
		for (const TPair<FName, float>& MorphTarget : Found->MorphTargets)
		{
			GetMesh()->SetMorphTarget(MorphTarget.Key, MorphTarget.Value);
		}
	}
}

bool APlayerMaidCharacter::CanTakeDamage() const 
{
	// the only reason fail doesn't take damage is because the montage is super long
	// at some point I should come up with failed actually doing damage in some way so there's some reward
	// and failing doesn't make it so that you just did 3 inputs for no reason :shrug:
	return !MeiDouComponent->IsPlayingResult() && !MeiDouComponent->IsFailed();
}