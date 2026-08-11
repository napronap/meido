// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/MeiDouPoseBubbleComponent.h"

#include "ActorComponents/MeiDouComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "UI/MeiDouPoseBubbleWidget.h"

UMeiDouPoseBubbleComponent::UMeiDouPoseBubbleComponent()
{
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetGenerateOverlapEvents(false);
	SetWidgetSpace(EWidgetSpace::Screen);
	SetDrawAtDesiredSize(true);
	SetPivot(FVector2D(0.5f, 1.f));
	// Slightly above capsule; author can offset in BP.
	SetRelativeLocation(FVector(0.f, 0.f, 100.f));
}

void UMeiDouPoseBubbleComponent::BeginPlay()
{
	if (BubbleWidgetClass)
	{
		SetWidgetClass(BubbleWidgetClass);
	}

	Super::BeginPlay();
	EnsureBubbleWidget();
	HidePoseBubble();

	if (bBindToOwnerMeiDouOnBeginPlay)
	{
		if (AActor* Owner = GetOwner())
		{
			BindToMeiDou(Owner->FindComponentByClass<UMeiDouComponent>());
		}
	}
}

void UMeiDouPoseBubbleComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearHideTimer();
	UnbindMeiDou();
	BubbleWidget = nullptr;
	Super::EndPlay(EndPlayReason);
}

void UMeiDouPoseBubbleComponent::EnsureBubbleWidget()
{
	if (BubbleWidget)
	{
		return;
	}

	if (!GetUserWidgetObject())
	{
		InitWidget();
	}

	BubbleWidget = Cast<UMeiDouPoseBubbleWidget>(GetUserWidgetObject());
}

void UMeiDouPoseBubbleComponent::BindToMeiDou(UMeiDouComponent* MeiDou)
{
	UnbindMeiDou();
	if (!MeiDou)
	{
		return;
	}

	BoundMeiDou = MeiDou;
	BoundMeiDou->OnPoseAnimationRequested.AddDynamic(
		this,
		&UMeiDouPoseBubbleComponent::HandlePoseRequested
	);
}

void UMeiDouPoseBubbleComponent::UnbindMeiDou()
{
	if (BoundMeiDou)
	{
		BoundMeiDou->OnPoseAnimationRequested.RemoveDynamic(
			this,
			&UMeiDouPoseBubbleComponent::HandlePoseRequested
		);
		BoundMeiDou = nullptr;
	}
}

void UMeiDouPoseBubbleComponent::HandlePoseRequested(const FMeiDouPoseAnimationRequest& Request)
{
	ShowPoseBubble(Request.Input);
}

void UMeiDouPoseBubbleComponent::ShowPoseBubble(const EMeiDouInput Input)
{
	EnsureBubbleWidget();
	if (!BubbleWidget)
	{
		return;
	}

	BubbleWidget->ShowPose(Input);
	SetVisibility(true);

	ClearHideTimer();
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			HideTimerHandle,
			this,
			&UMeiDouPoseBubbleComponent::HidePoseBubble,
			DisplayDuration,
			false
		);
	}
}

void UMeiDouPoseBubbleComponent::HidePoseBubble()
{
	ClearHideTimer();
	if (BubbleWidget)
	{
		BubbleWidget->HideBubble();
	}
	// Keep component active; widget collapsed.
}

void UMeiDouPoseBubbleComponent::ClearHideTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(HideTimerHandle);
	}
}
