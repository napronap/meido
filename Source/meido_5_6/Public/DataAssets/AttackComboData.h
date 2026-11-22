#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AttackComboData.generated.h"

class UAnimMontage;

UENUM(BlueprintType)
enum class EHitStopType : uint8
{
	Light UMETA(DisplayName = "Light"),
	Heavy UMETA(DisplayName = "Heavy")
};

USTRUCT(BlueprintType)
struct FAttackHit
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FName> Sockets;

	// half size of the box swept along each socket's path, oriented to the socket
	// 7,10,10 is an appropriate size for the maid's hand
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector BoxExtent = FVector(7.f, 10.f, 10.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Damage = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EHitStopType HitStopType = EHitStopType::Light;
};

// one attack of the combo chain
// needs notifies:
// AttackHitWindow
// CheckCombo (chain to next step)
// RecoveryEnd (when the character is allowed to move again)
USTRUCT(BlueprintType)
struct FAttackComboStep
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> Montage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAttackHit Hit;

	// with a MontageTimeStretchCurve in the montage the speed up (PlayRate > 1) goes where the curve is high,
	// without it the whole montage would speed up constantly
	// at 1 the curve is ignored. Enemy characters should always keep 1 since they share the same montages with the player
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0.1"))
	float PlayRate = 1.f;
};

// Melee combo
// steps are played in order
UCLASS()
class MEIDO_5_6_API UAttackComboData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FAttackComboStep> Steps;
};
