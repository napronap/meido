#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "DemoCameraManager.generated.h"

class APlayerMaidCharacter;

// How the camera frames the maid in a flow state: an orbit around her, and how long the camera takes to get there
USTRUCT(BlueprintType)
struct FFlowCameraShot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, meta=(ClampMin="0.0"))
	float ArmLength = 300.f;

	UPROPERTY(EditAnywhere)
	float Pitch = -10.f;

	// added to the maid's yaw: 0 is behind her, 180 in front
	UPROPERTY(EditAnywhere)
	float YawOffset = 0.f;

	// sideways, like the boom's socket offset Y
	UPROPERTY(EditAnywhere)
	float SideOffset = 0.f;

	// ignored by SnapTo
	UPROPERTY(EditAnywhere, meta=(ClampMin="0.01"))
	float BlendTime = 1.25f;
};

// Demo flow camera (menu, win, lose)
// While a shot is active this places the camera itself, orbiting around the pivot of the maid's camera boom (the boom is
// never moved). Blending the orbit (distance, rotation, side) instead of the location goes around her, not through her
// Without a shot the view is the maid's own spring arm camera
UCLASS()
class MEIDO_5_6_API ADemoCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()

public:
	void SnapTo(const FFlowCameraShot& Shot);
	void BlendTo(const FFlowCameraShot& Shot);

	// back to the maid's own camera, looking from behind her at Pitch
	void BlendToPawn(float Pitch, float BlendTime);

protected:
	virtual void UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime) override;

private:
	struct FOrbit
	{
		float ArmLength = 0.f;
		FRotator Rotation = FRotator::ZeroRotator;
		// double like the boom's SocketOffset (FVector)
		double SideOffset = 0.0;
	};

	// from the current camera (shot or spring arm) to Target; drops the lock-on, it would keep turning the strafing maid
	void StartBlend(const FOrbit& Target, float BlendTime, bool bToPawn);

	APlayerMaidCharacter* GetMaid() const;

	UPROPERTY(EditDefaultsOnly, Category="Flow", meta=(ClampMin="1.0"))
	float BlendExponent = 2.f;

	// the camera is placed here instead of by the spring arm
	bool bShotActive = false;
	// when the blend ends the view goes back to the spring arm
	bool bBlendingToPawn = false;

	FOrbit From;
	FOrbit To;
	FOrbit Current;
	float BlendElapsed = 0.f;
	float BlendDuration = 0.f;
};
