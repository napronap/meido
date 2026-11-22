#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PlayerInputConfig.generated.h"

class UInputAction;
class UInputMappingContext;

// The player's input: the mapping context and its actions
// Shared by every player controller (demo, sandbox), so a new action is assigned in one place
UCLASS()
class MEIDO_5_6_API UPlayerInputConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	// only using one IMC for now
	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputMappingContext> MappingContext;

	UPROPERTY(EditDefaultsOnly, Category="Input|Movement")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, Category="Input|Movement")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, Category="Input|Movement")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditDefaultsOnly, Category="Input|Movement")
	TObjectPtr<UInputAction> DashAction;

	UPROPERTY(EditDefaultsOnly, Category="Input|Combat")
	TObjectPtr<UInputAction> AttackAction;

	UPROPERTY(EditDefaultsOnly, Category="Input|MeiDou")
	TObjectPtr<UInputAction> MeiDouMoeAction;

	UPROPERTY(EditDefaultsOnly, Category="Input|MeiDou")
	TObjectPtr<UInputAction> MeiDouKyunAction;

	UPROPERTY(EditDefaultsOnly, Category="Input|MeiDou")
	TObjectPtr<UInputAction> MeiDouNyanAction;

	UPROPERTY(EditDefaultsOnly, Category="Input|LockOn")
	TObjectPtr<UInputAction> LockOnAction;

	UPROPERTY(EditDefaultsOnly, Category="Input|LockOn")
	TObjectPtr<UInputAction> SwitchLockOnAction;
};
