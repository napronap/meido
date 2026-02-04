// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorComponents/MeiDouComponent.h"

// Sets default values for this component's properties
UMeiDouComponent::UMeiDouComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


const TArray<EMeiDouInput>& UMeiDouComponent::GetInputBuffer() const
{
	return InputBuffer;
}

int32 UMeiDouComponent::GetInputCount(EMeiDouInput Input) const
{
	int32 Count = 0;

	for (int32 i = 0; i < InputBuffer.Num(); i++)
	{
		if (InputBuffer[i] == Input)
		{
			Count++;
		}
		else
		{
			break;
		}
	}
	
	return Count;
}

void UMeiDouComponent::BeginPlay()
{
	Super::BeginPlay();
	InputBuffer.Reserve(MaxInputs);
}

void UMeiDouComponent::RegisterInput(EMeiDouInput Input)
{
	if (InputBuffer.Num() >= MaxInputs)
	{
		return;
	}

	InputBuffer.Add(Input);

	RestartInputTimer();

	if (InputBuffer.Num() == MaxInputs)
	{
		ClearInputTimer();
		TryResolve();
	}
}

void UMeiDouComponent::ResetCombo()
{
	InputBuffer.Reset();
}

void UMeiDouComponent::TryResolve()
{
	if (!ComboData || InputBuffer.Num() != 3) return;

	FMeiDouComboKey Key;
	Key.A = InputBuffer[0];
	Key.B = InputBuffer[1];
	Key.C = InputBuffer[2];

	if (const FMeiDouComboDefinition* Def = ComboData->Combos.Find(Key))
	{
		FMeiDouResolvedCombo Result;
		Result.ComboId = Def->ComboId;
		Result.Inputs = InputBuffer;

		OnComboResolved.Broadcast(Result);
	}

	// if combo doesn't exist, do nothing and reset the input
	ResetCombo();
}

void UMeiDouComponent::RestartInputTimer()
{
	if (!GetWorld())
	{
		return;
	}

	GetWorld()->GetTimerManager().ClearTimer(InputTimeoutHandle);
	GetWorld()->GetTimerManager().SetTimer(
		InputTimeoutHandle,
		this,
		&UMeiDouComponent::OnInputTimeout,
		MaxInputGap,
		false
	);
}

void UMeiDouComponent::ClearInputTimer()
{
	if (!GetWorld())
	{
		return;
	}

	GetWorld()->GetTimerManager().ClearTimer(InputTimeoutHandle);
}

void UMeiDouComponent::OnInputTimeout()
{
	GEngine->AddOnScreenDebugMessage(
		-1,
		2.f,
		FColor::Yellow,
		TEXT("MeiDou timeout → buffer reset")
	);
	InputBuffer.Reset();
}
