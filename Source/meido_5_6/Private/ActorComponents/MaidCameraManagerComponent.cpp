// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorComponents/MaidCameraManagerComponent.h"
#include "ActorComponents/LockOnComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Interfaces/Targetable.h"

UMaidCameraManagerComponent::UMaidCameraManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UMaidCameraManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!CameraBoom || !ViewCamera)
	{
		if (AActor* Owner = GetOwner())
		{
			if (!CameraBoom)
			{
				CameraBoom = Owner->FindComponentByClass<USpringArmComponent>();
			}
			if (!ViewCamera)
			{
				ViewCamera = Owner->FindComponentByClass<UCameraComponent>();
			}
		}
	}

	if (!BoundLockOn)
	{
		if (AActor* Owner = GetOwner())
		{
			BindLockOn(Owner->FindComponentByClass<ULockOnComponent>());
		}
	}

	ApplyActiveProfileSettings();
}

void UMaidCameraManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (BoundLockOn)
	{
		BoundLockOn->OnLockOnChanged.RemoveDynamic(this, &UMaidCameraManagerComponent::HandleLockOnChanged);
		BoundLockOn = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

void UMaidCameraManagerComponent::Initialize(USpringArmComponent* InBoom, UCameraComponent* InCamera)
{
	CameraBoom = InBoom;
	ViewCamera = InCamera;
	ApplyActiveProfileSettings();
}

void UMaidCameraManagerComponent::BindLockOn(ULockOnComponent* InLockOn)
{
	if (BoundLockOn)
	{
		BoundLockOn->OnLockOnChanged.RemoveDynamic(this, &UMaidCameraManagerComponent::HandleLockOnChanged);
	}

	BoundLockOn = InLockOn;

	if (BoundLockOn)
	{
		BoundLockOn->OnLockOnChanged.AddDynamic(this, &UMaidCameraManagerComponent::HandleLockOnChanged);
		if (BoundLockOn->IsLockedOn())
		{
			SetProfile(EMaidCameraProfile::LockOn);
		}
	}
}

void UMaidCameraManagerComponent::SetProfile(const EMaidCameraProfile NewProfile)
{
	if (ActiveProfile == NewProfile)
	{
		ApplyActiveProfileSettings();
		return;
	}

	ActiveProfile = NewProfile;
	ApplyActiveProfileSettings();
}

void UMaidCameraManagerComponent::ApplyActiveProfileSettings()
{
	if (!CameraBoom)
	{
		return;
	}

	CameraBoom->bDoCollisionTest = bDoCameraCollision;
	CameraBoom->ProbeSize = ProbeSize;
	CameraBoom->ProbeChannel = ProbeChannel;

	switch (ActiveProfile)
	{
	case EMaidCameraProfile::LockOn:
		CameraBoom->TargetArmLength = LockOnArmLength;
		CameraBoom->bEnableCameraRotationLag = bLockOnEnableRotationLag;
		CameraBoom->bEnableCameraLag = bLockOnEnablePositionLag;
		CameraBoom->CameraLagSpeed = LockOnPositionLagSpeed;
		if (bLockOnEnableRotationLag)
		{
			CameraBoom->CameraRotationLagSpeed = FreeRotationLagSpeed;
		}
		break;

	case EMaidCameraProfile::Free:
	default:
		CameraBoom->TargetArmLength = FreeArmLength;
		CameraBoom->bEnableCameraRotationLag = bFreeEnableRotationLag;
		CameraBoom->CameraRotationLagSpeed = FreeRotationLagSpeed;
		CameraBoom->bEnableCameraLag = bFreeEnablePositionLag;
		CameraBoom->CameraLagSpeed = FreePositionLagSpeed;
		break;
	}
}

void UMaidCameraManagerComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction
)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (ActiveProfile == EMaidCameraProfile::LockOn)
	{
		UpdateLockOnCamera(DeltaTime);
	}
}

void UMaidCameraManagerComponent::UpdateLockOnCamera(const float DeltaTime)
{
	if (!BoundLockOn || !BoundLockOn->IsLockedOn())
	{
		return;
	}

	if (AActor* Target = BoundLockOn->GetCurrentTarget())
	{
		RotateControlYawTowardTarget(Target, DeltaTime);
	}
}

void UMaidCameraManagerComponent::RotateControlYawTowardTarget(AActor* Target, const float DeltaTime)
{
	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn || !ViewCamera || !Target)
	{
		return;
	}

	AController* Controller = OwnerPawn->GetController();
	if (!Controller)
	{
		return;
	}

	FVector TargetLocation;
	if (Target->GetClass()->ImplementsInterface(UTargetable::StaticClass()))
	{
		TargetLocation = ITargetable::Execute_GetTargetLocation(Target);
	}
	else
	{
		TargetLocation = Target->GetActorLocation();
	}

	const FVector CameraLocation = ViewCamera->GetComponentLocation();
	const FVector Direction = (TargetLocation - CameraLocation).GetSafeNormal();
	if (Direction.IsNearlyZero())
	{
		return;
	}

	const FRotator DesiredRotation = Direction.Rotation();
	const FRotator CurrentRotation = Controller->GetControlRotation();
	const FRotator DesiredControlRotation(
		CurrentRotation.Pitch,
		DesiredRotation.Yaw,
		CurrentRotation.Roll
	);

	const FRotator NewRotation = FMath::RInterpTo(
		CurrentRotation,
		DesiredControlRotation,
		DeltaTime,
		LockOnYawInterpSpeed
	);

	Controller->SetControlRotation(NewRotation);
}

void UMaidCameraManagerComponent::HandleLockOnChanged(AActor* NewTarget, const bool bSuccess)
{
	const bool bLocked = bSuccess && NewTarget != nullptr;
	SetProfile(bLocked ? EMaidCameraProfile::LockOn : EMaidCameraProfile::Free);
}
