// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorComponents/LockOnComponent.h"

#include "Algo/Sort.h"
#include "Blueprint/UserWidget.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Interfaces/Targetable.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UObject/ConstructorHelpers.h"

ULockOnComponent::ULockOnComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	TargetObjectType.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

	// Default Content marker if present (author can override / clear on the native component).
	static ConstructorHelpers::FClassFinder<UUserWidget> DefaultMarker(
		TEXT("/Game/Blueprints/Widgets/WBP_LockOnMarker")
	);
	if (DefaultMarker.Succeeded())
	{
		MarkerWidgetClass = DefaultMarker.Class;
	}
}

void ULockOnComponent::BeginPlay()
{
	Super::BeginPlay();
	OwnerCharacter = Cast<ACharacter>(GetOwner());
}

void ULockOnComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	DestroyMarkerWidget();
	CurrentTarget = nullptr;
	Super::EndPlay(EndPlayReason);
}

void ULockOnComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction
)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateMarkerScreenPosition();
}

void ULockOnComponent::FindTargets(TArray<AActor*>& OutTargets) const
{
	OutTargets.Reset();

	if (!OwnerCharacter || !GetWorld())
	{
		return;
	}

	if (TargetObjectType.Num() == 0)
	{
		return;
	}

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

	for (AActor* Actor : Overlaps)
	{
		if (!Actor)
		{
			continue;
		}

		if (Actor->GetClass()->ImplementsInterface(UTargetable::StaticClass())
			&& ITargetable::Execute_CanBeTargeted(Actor))
		{
			OutTargets.Add(Actor);
		}
	}
}

FVector ULockOnComponent::ResolveTargetWorldLocation(const AActor* Target)
{
	if (!Target)
	{
		return FVector::ZeroVector;
	}

	if (Target->GetClass()->ImplementsInterface(UTargetable::StaticClass()))
	{
		return ITargetable::Execute_GetTargetLocation(const_cast<AActor*>(Target));
	}

	return Target->GetActorLocation();
}

APlayerController* ULockOnComponent::GetOwnerPlayerController() const
{
	if (!OwnerCharacter)
	{
		return nullptr;
	}
	return Cast<APlayerController>(OwnerCharacter->GetController());
}

AActor* ULockOnComponent::SelectBestTarget(const TArray<AActor*>& Targets, APlayerController* PC) const
{
	if (!PC || Targets.Num() == 0)
	{
		return nullptr;
	}

	int32 ViewportX = 0;
	int32 ViewportY = 0;
	PC->GetViewportSize(ViewportX, ViewportY);
	if (ViewportX <= 0 || ViewportY <= 0)
	{
		return nullptr;
	}

	const FVector2D ScreenCenter(ViewportX * 0.5f, ViewportY * 0.5f);

	AActor* BestTarget = nullptr;
	float BestDistSq = TNumericLimits<float>::Max();

	for (AActor* Target : Targets)
	{
		if (!Target)
		{
			continue;
		}

		const FVector TargetLocation = ResolveTargetWorldLocation(Target);

		FVector2D ScreenPos;
		if (!PC->ProjectWorldLocationToScreen(TargetLocation, ScreenPos))
		{
			continue;
		}

		if (ScreenPos.X < 0.f || ScreenPos.Y < 0.f
			|| ScreenPos.X > ViewportX || ScreenPos.Y > ViewportY)
		{
			continue;
		}

		const float DistSq = FVector2D::DistSquared(ScreenPos, ScreenCenter);
		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			BestTarget = Target;
		}
	}

	return BestTarget;
}

bool ULockOnComponent::TryLockOn()
{
	if (!OwnerCharacter)
	{
		BroadcastLockOnChanged(nullptr, false);
		return false;
	}

	APlayerController* PC = GetOwnerPlayerController();
	if (!PC)
	{
		BroadcastLockOnChanged(nullptr, false);
		return false;
	}

	TArray<AActor*> Candidates;
	FindTargets(Candidates);

	AActor* BestTarget = SelectBestTarget(Candidates, PC);
	if (!BestTarget)
	{
		BroadcastLockOnChanged(nullptr, false);
		return false;
	}

	CurrentTarget = BestTarget;
	bCanSwitchTarget = true;
	BroadcastLockOnChanged(BestTarget, true);

	if (bDrawDebugLockOn && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.2f, FColor::Cyan, TEXT("LockOn: acquired"));
	}

	return true;
}

void ULockOnComponent::ClearLockOn()
{
	const bool bHadTarget = CurrentTarget != nullptr;
	CurrentTarget = nullptr;
	bCanSwitchTarget = true;
	BroadcastLockOnChanged(nullptr, false);

	if (bHadTarget && bDrawDebugLockOn && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.2f, FColor::Cyan, TEXT("LockOn: cleared"));
	}
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

void ULockOnComponent::HandleSwitchInput(const float AxisValue)
{
	if (!IsLockedOn())
	{
		return;
	}

	if (!bCanSwitchTarget)
	{
		if (FMath::Abs(AxisValue) <= SwitchInputDeadZone)
		{
			bCanSwitchTarget = true;
		}
		return;
	}

	if (AxisValue > SwitchInputDeadZone)
	{
		SwitchTarget(+1);
		bCanSwitchTarget = false;
	}
	else if (AxisValue < -SwitchInputDeadZone)
	{
		SwitchTarget(-1);
		bCanSwitchTarget = false;
	}
}

void ULockOnComponent::HandleSwitchReleased()
{
	bCanSwitchTarget = true;
}

void ULockOnComponent::SwitchTarget(const int32 Direction)
{
	if (!RefreshCurrentTarget() || !OwnerCharacter || !CurrentTarget)
	{
		return;
	}

	APlayerController* PC = GetOwnerPlayerController();
	if (!PC)
	{
		return;
	}

	TArray<AActor*> Candidates;
	FindTargets(Candidates);
	if (Candidates.Num() <= 1)
	{
		return;
	}

	int32 ViewportX = 0;
	int32 ViewportY = 0;
	PC->GetViewportSize(ViewportX, ViewportY);

	struct FScreenTarget
	{
		AActor* Actor = nullptr;
		FVector2D ScreenPos = FVector2D::ZeroVector;
	};

	TArray<FScreenTarget> Visible;
	Visible.Reserve(Candidates.Num());

	for (AActor* Target : Candidates)
	{
		if (!Target)
		{
			continue;
		}

		const FVector WorldPos = ResolveTargetWorldLocation(Target);
		FVector2D ScreenPos;
		if (!PC->ProjectWorldLocationToScreen(WorldPos, ScreenPos))
		{
			continue;
		}

		if (ScreenPos.X < 0.f || ScreenPos.X > ViewportX
			|| ScreenPos.Y < 0.f || ScreenPos.Y > ViewportY)
		{
			continue;
		}

		Visible.Add({Target, ScreenPos});
	}

	if (Visible.Num() <= 1)
	{
		return;
	}

	Algo::Sort(Visible, [](const FScreenTarget& A, const FScreenTarget& B)
	{
		return A.ScreenPos.X < B.ScreenPos.X;
	});

	int32 CurrentIndex = INDEX_NONE;
	for (int32 i = 0; i < Visible.Num(); ++i)
	{
		if (Visible[i].Actor == CurrentTarget)
		{
			CurrentIndex = i;
			break;
		}
	}

	if (CurrentIndex == INDEX_NONE)
	{
		return;
	}

	const int32 Step = (Direction > 0) ? +1 : -1;
	int32 NextIndex = CurrentIndex + Step;
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
		BroadcastLockOnChanged(CurrentTarget, true);
		if (bDrawDebugLockOn && GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Cyan, TEXT("LockOn: switched"));
		}
	}
}

bool ULockOnComponent::IsTargetValidForLockOn(const AActor* Target) const
{
	if (!OwnerCharacter || !IsValid(Target) || Target == OwnerCharacter)
	{
		return false;
	}

	if (!Target->GetClass()->ImplementsInterface(UTargetable::StaticClass()))
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

void ULockOnComponent::BroadcastLockOnChanged(AActor* NewTarget, const bool bSuccess)
{
	OnLockOnChanged.Broadcast(NewTarget, bSuccess);
	ApplyMarkerForLockState(NewTarget, bSuccess);
}

void ULockOnComponent::ApplyMarkerForLockState(AActor* NewTarget, const bool bSuccess)
{
	const bool bShow = bShowLockOnMarker && bSuccess && NewTarget != nullptr && MarkerWidgetClass;

	if (!bShow)
	{
		DestroyMarkerWidget();
		SetComponentTickEnabled(false);
		return;
	}

	APlayerController* PC = GetOwnerPlayerController();
	if (!PC)
	{
		DestroyMarkerWidget();
		SetComponentTickEnabled(false);
		return;
	}

	if (!ActiveMarkerWidget)
	{
		ActiveMarkerWidget = CreateWidget<UUserWidget>(PC, MarkerWidgetClass);
		if (ActiveMarkerWidget)
		{
			ActiveMarkerWidget->AddToViewport(50);
			ActiveMarkerWidget->SetAlignmentInViewport(FVector2D(0.5f, 0.5f));
			ActiveMarkerWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
	}

	if (ActiveMarkerWidget)
	{
		SetComponentTickEnabled(true);
		UpdateMarkerScreenPosition();
	}
	else
	{
		SetComponentTickEnabled(false);
	}
}

void ULockOnComponent::DestroyMarkerWidget()
{
	if (ActiveMarkerWidget)
	{
		ActiveMarkerWidget->RemoveFromParent();
		ActiveMarkerWidget = nullptr;
	}
}

void ULockOnComponent::UpdateMarkerScreenPosition()
{
	if (!ActiveMarkerWidget || !CurrentTarget)
	{
		return;
	}

	APlayerController* PC = GetOwnerPlayerController();
	if (!PC)
	{
		return;
	}

	const FVector WorldPos = ResolveTargetWorldLocation(CurrentTarget);
	FVector2D ScreenPos;
	if (!PC->ProjectWorldLocationToScreen(WorldPos, ScreenPos))
	{
		ActiveMarkerWidget->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	ActiveMarkerWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
	ActiveMarkerWidget->SetPositionInViewport(ScreenPos + MarkerScreenOffset, true);
}
