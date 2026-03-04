// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorComponents/LockOnComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Interfaces/Targetable.h"
#include "Algo/Sort.h" // arriba en el cpp

// Sets default values for this component's properties
ULockOnComponent::ULockOnComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


// Called when the game starts
void ULockOnComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ACharacter>(GetOwner());
}

void ULockOnComponent::FindTargets(TArray<AActor*>& OutTargets) const
{
	TArray<AActor*> Overlaps;

	UKismetSystemLibrary::SphereOverlapActors(
		GetWorld(),
		OwnerCharacter->GetActorLocation(),
		SearchRadius,
		TargetObjectType,
		nullptr,
		{OwnerCharacter},
		Overlaps
	);

	FString Message1 = FString::Printf(
		TEXT("Overlaps found: %d"), Overlaps.Num());

	GEngine->AddOnScreenDebugMessage(
		-1, 1.5f, FColor::Yellow, Message1
	);

	UE_LOG(LogTemp, Warning, TEXT("Overlaps found: %d"), Overlaps.Num());

	for (AActor* Actor : Overlaps)
	{
		if (!Actor) continue;

		if (Actor->Implements<UTargetable>())
		{
			const bool bCanBeTargeted = ITargetable::Execute_CanBeTargeted(Actor);

			if (bCanBeTargeted)
			{
				OutTargets.Add(Actor);
			}
		}

		FString Message2 = FString::Printf(
			TEXT("Overlap: %s, ImplementsTargetable=%d"), *GetNameSafe(Actor),
			Actor && Actor->Implements<UTargetable>());

		GEngine->AddOnScreenDebugMessage(
			-1, 1.5f, FColor::Yellow, Message2
		);
	}
}

bool ULockOnComponent::TryLockOn()
{
	if (!OwnerCharacter)
	{
		OnLockOnChanged.Broadcast(nullptr, false);
		return false;
	}

	APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController());
	if (!PC)
	{
		OnLockOnChanged.Broadcast(nullptr, false);
		return false;
	}

	TArray<AActor*> Candidates;
	FindTargets(Candidates);

	if (Candidates.Num() == 0)
	{
		OnLockOnChanged.Broadcast(nullptr, false);
		return false;
	}

	int32 ViewportX, ViewportY;
	PC->GetViewportSize(ViewportX, ViewportY);

	const FVector2D ScreenCenter(
		ViewportX * 0.5f,
		ViewportY * 0.5f
	);

	AActor* BestTarget = nullptr;
	float BestDistSq = TNumericLimits<float>::Max();

	for (AActor* Target : Candidates)
	{
		FVector TargetLocation = Target->GetActorLocation();


		// Si implementa ITargetable, usamos su punto de target
		if (Target->Implements<UTargetable>())
		{
			TargetLocation = ITargetable::Execute_GetTargetLocation(Target);
		}

		FVector2D ScreenPos;
		const bool bOnScreen =
			PC->ProjectWorldLocationToScreen(TargetLocation, ScreenPos);

		if (!bOnScreen)
			continue;

		// Opcional: margen para evitar bordes extremos
		if (ScreenPos.X < 0.f || ScreenPos.Y < 0.f ||
			ScreenPos.X > ViewportX || ScreenPos.Y > ViewportY)
			continue;

		const float DistSq =
			FVector2D::DistSquared(ScreenPos, ScreenCenter);

		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			BestTarget = Target;
		}
	}

	if (BestTarget)
	{
		CurrentTarget = BestTarget;
		bCanSwitchTarget = true; // reset
		OnLockOnChanged.Broadcast(BestTarget, true);
		return true;
	}

	OnLockOnChanged.Broadcast(nullptr, false);
	return false;
}


void ULockOnComponent::ClearLockOn()
{
	CurrentTarget = nullptr;
	bCanSwitchTarget = true;
	OnLockOnChanged.Broadcast(nullptr, false);
}

bool ULockOnComponent::IsLockedOn()
{
	return RefreshCurrentTarget();
}

AActor* ULockOnComponent::GetCurrentTarget()
{
	if (!RefreshCurrentTarget())
	{
		return nullptr;
	}

	return CurrentTarget;
}

void ULockOnComponent::HandleSwitchInput(float AxisValue)
{
	if (!IsLockedOn())
		return;

	if (!bCanSwitchTarget)
	{
		if (FMath::Abs(AxisValue) < NeutralThreshold)
		{
			bCanSwitchTarget = true;
		}
		return;
	}

	if (AxisValue > SwitchThreshold)
	{
		SwitchTarget(+1);
		bCanSwitchTarget = false;
	}
	else if (AxisValue < -SwitchThreshold)
	{
		SwitchTarget(-1);
		bCanSwitchTarget = false;
	}
}

void ULockOnComponent::HandleSwitchReleased()
{
	bCanSwitchTarget = true;
}



void ULockOnComponent::SwitchTarget(int32 Direction)
{
	if (!RefreshCurrentTarget() || !OwnerCharacter || !CurrentTarget)
		return;

	APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController());
	if (!PC)
		return;

	TArray<AActor*> Candidates;
	FindTargets(Candidates);

	if (Candidates.Num() <= 1)
		return;

	int32 ViewportX, ViewportY;
	PC->GetViewportSize(ViewportX, ViewportY);

	struct FScreenTarget
	{
		AActor* Actor = nullptr;
		FVector2D ScreenPos = FVector2D::ZeroVector;
	};

	TArray<FScreenTarget> Visible;

	Visible.Reserve(Candidates.Num());

	auto GetTargetWorldPos = [](AActor* Actor) -> FVector
	{
		if (!Actor) return FVector::ZeroVector;

		if (Actor->Implements<UTargetable>())
		{
			return ITargetable::Execute_GetTargetLocation(Actor);
		}

		return Actor->GetActorLocation();
	};

	// 1) Proyectar todos y quedarse con los que están en pantalla
	for (AActor* Target : Candidates)
	{
		if (!Target) continue;

		const FVector WorldPos = GetTargetWorldPos(Target);

		FVector2D ScreenPos;
		if (!PC->ProjectWorldLocationToScreen(WorldPos, ScreenPos))
			continue;

		// on-screen check
		if (ScreenPos.X < 0.f || ScreenPos.X > ViewportX ||
			ScreenPos.Y < 0.f || ScreenPos.Y > ViewportY)
			continue;

		Visible.Add({Target, ScreenPos});
	}

	if (Visible.Num() <= 1)
		return;

	// 2) Ordenar por X (izquierda → derecha)
	Algo::Sort(Visible, [](const FScreenTarget& A, const FScreenTarget& B)
	{
		return A.ScreenPos.X < B.ScreenPos.X;
	});

	// 3) Encontrar el índice del current
	int32 CurrentIndex = INDEX_NONE;
	for (int32 i = 0; i < Visible.Num(); ++i)
	{
		if (Visible[i].Actor == CurrentTarget)
		{
			CurrentIndex = i;
			break;
		}
	}

	// Si el current no está en la lista visible (raro pero puede pasar), no switch
	if (CurrentIndex == INDEX_NONE)
		return;

	// 4) Elegir vecino: Direction +1 = derecha, -1 = izquierda
	const int32 Step = (Direction > 0) ? +1 : -1;
	int32 NextIndex = CurrentIndex + Step;

	// Si no querés wrap, dejalo así:
	if (NextIndex < 0)
	{
		NextIndex = Visible.Num() - 1;
	}
	else if (NextIndex >= Visible.Num())
	{
		NextIndex = 0;
	}

	AActor* PreviousTarget = CurrentTarget;
	CurrentTarget = Visible[NextIndex].Actor;
	if (CurrentTarget != PreviousTarget)
	{
		OnLockOnChanged.Broadcast(CurrentTarget, true);
	}
}

bool ULockOnComponent::IsTargetValidForLockOn(const AActor* Target) const
{
	if (!OwnerCharacter || !IsValid(Target) || Target == OwnerCharacter)
	{
		return false;
	}

	if (!Target->Implements<UTargetable>())
	{
		return false;
	}

	AActor* MutableTarget = const_cast<AActor*>(Target);
	return IsValid(MutableTarget) && ITargetable::Execute_CanBeTargeted(MutableTarget);
}

bool ULockOnComponent::RefreshCurrentTarget()
{
	if (!CurrentTarget)
	{
		return false;
	}

	if (IsTargetValidForLockOn(CurrentTarget))
	{
		return true;
	}

	CurrentTarget = nullptr;
	return TryLockOn();
}


// Called every frame
void ULockOnComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                     FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}
