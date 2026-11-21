#include "GameModes/DemoCameraManager.h"
#include "ActorComponents/LockOnComponent.h"
#include "Characters/PlayerMaidCharacter.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"

void ADemoCameraManager::SnapTo(const FFlowCameraShot& Shot)
{
	BlendTo(Shot);
	BlendDuration = 0.f;
}

void ADemoCameraManager::BlendTo(const FFlowCameraShot& Shot)
{
	const FRotator Rotation(Shot.Pitch, GetMaid()->GetActorRotation().Yaw + Shot.YawOffset, 0.f);
	StartBlend({Shot.ArmLength, Rotation, Shot.SideOffset}, Shot.BlendTime, false);
}

void ADemoCameraManager::BlendToPawn(const float Pitch, const float BlendTime)
{
	// the spring arm keeps its own length and offset, only the rotation changes
	const USpringArmComponent* CameraBoom = GetMaid()->GetCameraBoom();
	const FRotator Rotation(Pitch, GetMaid()->GetActorRotation().Yaw, 0.f);
	StartBlend({CameraBoom->TargetArmLength, Rotation, CameraBoom->SocketOffset.Y}, BlendTime, true);

	// the maid's camera already looks from there when the view is handed back to it
	PCOwner->SetControlRotation(Rotation);
}

void ADemoCameraManager::StartBlend(const FOrbit& Target, const float BlendTime, const bool bToPawn)
{
	ULockOnComponent* LockOnComponent = GetMaid()->GetLockOnComponent();
	if (LockOnComponent->IsLockedOn())
	{
		LockOnComponent->ClearLockOn();
	}

	if (bShotActive)
	{
		From = Current;
	}
	else
	{
		// where the spring arm has the camera now
		const USpringArmComponent* CameraBoom = GetMaid()->GetCameraBoom();
		From = {CameraBoom->TargetArmLength, PCOwner->GetControlRotation(), CameraBoom->SocketOffset.Y};
	}

	To = Target;
	BlendElapsed = 0.f;
	BlendDuration = BlendTime;
	bShotActive = true;
	bBlendingToPawn = bToPawn;
}

void ADemoCameraManager::UpdateViewTarget(FTViewTarget& OutVT, const float DeltaTime)
{
	Super::UpdateViewTarget(OutVT, DeltaTime);

	if (bShotActive)
	{
		BlendElapsed += DeltaTime;
		const float Alpha = BlendDuration > 0.f ? FMath::Min(BlendElapsed / BlendDuration, 1.f) : 1.f;
		const float EasedAlpha = FMath::InterpEaseInOut(0.f, 1.f, Alpha, BlendExponent);

		Current.ArmLength = FMath::Lerp(From.ArmLength, To.ArmLength, EasedAlpha);
		Current.Rotation = FMath::Lerp(From.Rotation, To.Rotation, EasedAlpha);
		Current.SideOffset = FMath::Lerp(From.SideOffset, To.SideOffset, EasedAlpha);

		// what the spring arm does, without its collision probe (its socket offset X / Z are 0)
		const USpringArmComponent* CameraBoom = GetMaid()->GetCameraBoom();
		const FVector Pivot = CameraBoom->GetComponentLocation() + CameraBoom->TargetOffset;
		OutVT.POV.Location = Pivot + Current.Rotation.RotateVector(FVector(-Current.ArmLength, Current.SideOffset, 0.f));
		OutVT.POV.Rotation = Current.Rotation;

		// arrived where the spring arm has the camera: from the next frame it's the maid's camera again
		if (bBlendingToPawn && Alpha >= 1.f)
		{
			bShotActive = false;
		}
	}
}

APlayerMaidCharacter* ADemoCameraManager::GetMaid() const
{
	return CastChecked<APlayerMaidCharacter>(PCOwner->GetPawn());
}
