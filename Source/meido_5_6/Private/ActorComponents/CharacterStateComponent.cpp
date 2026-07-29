// Fill out your copyright notice in the Description page of Project Settings.

#include "ActorComponents/CharacterStateComponent.h"
#include "Engine/Engine.h"

UCharacterStateComponent::UCharacterStateComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UCharacterStateComponent::BeginPlay()
{
	Super::BeginPlay();
	RecalculateOverall();
}

void UCharacterStateComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction
)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bDrawDebugState || !GEngine)
	{
		return;
	}

	const AActor* Owner = GetOwner();
	const FString OwnerName = Owner ? Owner->GetName() : TEXT("?");

	const UEnum* OverallEnum = StaticEnum<ECharacterOverallState>();
	const UEnum* AttackEnum = StaticEnum<EAttackState>();
	const UEnum* HealthEnum = StaticEnum<EHealthActionState>();

	const FString OverallStr = OverallEnum
		? OverallEnum->GetNameStringByValue(static_cast<int64>(OverallState))
		: TEXT("?");
	const FString AttackStr = AttackEnum
		? AttackEnum->GetNameStringByValue(static_cast<int64>(AttackState))
		: TEXT("?");
	const FString HealthStr = HealthEnum
		? HealthEnum->GetNameStringByValue(static_cast<int64>(HealthState))
		: TEXT("?");

	// Key by owner so multiple maids don't fight one debug line.
	const int32 DebugKey = Owner ? static_cast<int32>(GetTypeHash(OwnerName) & 0x7fffffff) : 0;
	GEngine->AddOnScreenDebugMessage(
		DebugKey,
		0.f,
		FColor::Cyan,
		FString::Printf(
			TEXT("[State] %s | Overall=%s Attack=%s Health=%s"),
			*OwnerName,
			*OverallStr,
			*AttackStr,
			*HealthStr
		)
	);
}

void UCharacterStateComponent::SetAttackState(const EAttackState NewState)
{
	if (AttackState == NewState)
	{
		return;
	}

	AttackState = NewState;
	RecalculateOverall();
}

void UCharacterStateComponent::SetHealthState(const EHealthActionState NewState)
{
	if (HealthState == NewState)
	{
		return;
	}

	HealthState = NewState;
	RecalculateOverall();
}

void UCharacterStateComponent::SetLocomotionState(const ELocomotionState NewState)
{
	if (LocomotionState == NewState)
	{
		return;
	}

	LocomotionState = NewState;
	RecalculateOverall();
}

void UCharacterStateComponent::SetMeiDouLayerState(const EMeiDouLayerState NewState)
{
	if (MeiDouLayerState == NewState)
	{
		return;
	}

	MeiDouLayerState = NewState;
	RecalculateOverall();
}

void UCharacterStateComponent::ResetToDefaults()
{
	AttackState = EAttackState::None;
	HealthState = EHealthActionState::Alive;
	LocomotionState = ELocomotionState::Grounded;
	MeiDouLayerState = EMeiDouLayerState::Idle;
	RecalculateOverall();
}

void UCharacterStateComponent::RecalculateOverall()
{
	// Priority: Dead > Stagger > MeiDouFailed > MeiDou > Attacking > Dashing > Jumping > Idle
	ECharacterOverallState NewOverall = ECharacterOverallState::Idle;

	if (HealthState == EHealthActionState::Dead)
	{
		NewOverall = ECharacterOverallState::Dead;
	}
	else if (HealthState == EHealthActionState::Stagger)
	{
		NewOverall = ECharacterOverallState::Stagger;
	}
	else if (MeiDouLayerState == EMeiDouLayerState::Failed)
	{
		NewOverall = ECharacterOverallState::MeiDouFailed;
	}
	else if (MeiDouLayerState == EMeiDouLayerState::Active)
	{
		NewOverall = ECharacterOverallState::MeiDou;
	}
	else if (AttackState != EAttackState::None)
	{
		NewOverall = ECharacterOverallState::Attacking;
	}
	else if (LocomotionState == ELocomotionState::Dash)
	{
		NewOverall = ECharacterOverallState::Dashing;
	}
	// Falling branch is ready if we ever SetLocomotionState(Falling); currently unused — see StateTypes.h.
	else if (LocomotionState == ELocomotionState::Jump
		|| LocomotionState == ELocomotionState::Falling)
	{
		NewOverall = ECharacterOverallState::Jumping;
	}
	else
	{
		NewOverall = ECharacterOverallState::Idle;
	}

	SetOverallInternal(NewOverall);
}

void UCharacterStateComponent::SetOverallInternal(const ECharacterOverallState NewOverall)
{
	if (OverallState == NewOverall)
	{
		return;
	}

	const ECharacterOverallState OldOverall = OverallState;
	OverallState = NewOverall;
	OnOverallStateChanged.Broadcast(OldOverall, NewOverall);
}
