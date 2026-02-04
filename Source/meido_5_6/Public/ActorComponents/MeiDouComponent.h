// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MeiDouComboData.h"
#include "Components/ActorComponent.h"
#include "Types/MeiDouTypes.h"
#include "MeiDouComponent.generated.h"

USTRUCT(BlueprintType)
struct FMeiDouResolvedCombo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FName ComboId;

	UPROPERTY(BlueprintReadOnly)
	TArray<EMeiDouInput> Inputs;
};

// APPARENTLY I need to set it as dynamic so that later on I can do stuff on BP
// I'm not sure if I'll have to, but let's keep it like this for now 
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnMeiDouComboResolved,
	const FMeiDouResolvedCombo&, Result
);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MEIDO_5_6_API UMeiDouComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UMeiDouComponent();

	// register the pose input
	void RegisterInput(EMeiDouInput Input);

	// reset the combo upon completion, damage, etc
	void ResetCombo();

	// Delegate that broadcasts the output of the combo
	// again, not sure if this will eventually be used in blueprints anyways
	UPROPERTY(BlueprintAssignable)
	FOnMeiDouComboResolved OnComboResolved;

	UPROPERTY(EditDefaultsOnly)
	UMeiDouComboData* ComboData;
	
	const TArray<EMeiDouInput>& GetInputBuffer() const;
	
	// Will return the amount of times the input argument is inside the current buffer
	int32 GetInputCount(EMeiDouInput Input) const;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	TArray<EMeiDouInput> InputBuffer;
	
	// no plans of making this have more than 3 inputs
	int32 MaxInputs = 3;
	float MaxInputGap = 1.f;
	FTimerHandle InputTimeoutHandle;

	void TryResolve();
	void RestartInputTimer();
	void ClearInputTimer();

	// functions which are called, in this case, by an unreal timer need to be registered in the reflection system
	UFUNCTION()
	void OnInputTimeout();
};
