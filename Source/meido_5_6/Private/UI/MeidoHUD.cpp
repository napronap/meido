// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/MeidoHUD.h"

#include "ActorComponents/HealthComponent.h"
#include "ActorComponents/MeiDouComponent.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UMeidoHUD::BindToHealth(UHealthComponent* Health)
{
	UnbindHealth();

	if (!Health)
	{
		return;
	}

	BoundHealth = Health;
	BoundHealth->OnHealthChanged.AddDynamic(this, &UMeidoHUD::HandleHealthChanged);
	ApplyPercentFromHealth(BoundHealth->GetCurrentHealth(), BoundHealth->GetMaxHealth());
}

void UMeidoHUD::UnbindHealth()
{
	if (BoundHealth)
	{
		BoundHealth->OnHealthChanged.RemoveDynamic(this, &UMeidoHUD::HandleHealthChanged);
		BoundHealth = nullptr;
	}
}

void UMeidoHUD::SetHealthPercent(const float Percent)
{
	if (UProgressBar* const Bar = HealthBar.Get())
	{
		Bar->SetPercent(FMath::Clamp(Percent, 0.f, 1.f));
	}
}

void UMeidoHUD::BindToMeiDou(UMeiDouComponent* MeiDou)
{
	UnbindMeiDou();

	if (!MeiDou)
	{
		ApplyMeiDouBuffer(TArray<EMeiDouInput>(), EMeiDouBufferChangeReason::ClearedOther);
		return;
	}

	BoundMeiDou = MeiDou;
	BoundMeiDou->OnMeiDouBufferChanged.AddDynamic(this, &UMeidoHUD::HandleMeiDouBufferChanged);
	ApplyMeiDouBuffer(BoundMeiDou->GetInputBuffer(), EMeiDouBufferChangeReason::ClearedOther);
}

void UMeidoHUD::UnbindMeiDou()
{
	if (BoundMeiDou)
	{
		BoundMeiDou->OnMeiDouBufferChanged.RemoveDynamic(this, &UMeidoHUD::HandleMeiDouBufferChanged);
		BoundMeiDou = nullptr;
	}
}

void UMeidoHUD::ApplyMeiDouBuffer(
	const TArray<EMeiDouInput>& Buffer,
	const EMeiDouBufferChangeReason Reason
)
{
	const EMeiDouInput SlotInputs[3] = {
		Buffer.IsValidIndex(0) ? Buffer[0] : EMeiDouInput::None,
		Buffer.IsValidIndex(1) ? Buffer[1] : EMeiDouInput::None,
		Buffer.IsValidIndex(2) ? Buffer[2] : EMeiDouInput::None
	};

	SetSlotText(MeiDouSlot0.Get(), SlotInputs[0], Buffer.IsValidIndex(0));
	SetSlotText(MeiDouSlot1.Get(), SlotInputs[1], Buffer.IsValidIndex(1));
	SetSlotText(MeiDouSlot2.Get(), SlotInputs[2], Buffer.IsValidIndex(2));

	OnMeiDouQueueUpdated(Buffer, Reason);
}

void UMeidoHUD::OnMeiDouQueueUpdated_Implementation(
	const TArray<EMeiDouInput>& /*Buffer*/,
	const EMeiDouBufferChangeReason /*Reason*/
)
{
	// BP: icons, flash on ClearedFail / ClearedTimeout
}

void UMeidoHUD::HandleHealthChanged(
	UHealthComponent* /*HealthComponent*/,
	const float CurrentHealth,
	const float MaxHealth,
	const float /*Delta*/
)
{
	ApplyPercentFromHealth(CurrentHealth, MaxHealth);
}

void UMeidoHUD::HandleMeiDouBufferChanged(
	const TArray<EMeiDouInput>& Buffer,
	const EMeiDouBufferChangeReason Reason
)
{
	ApplyMeiDouBuffer(Buffer, Reason);
}

void UMeidoHUD::ApplyPercentFromHealth(const float CurrentHealth, const float MaxHealth)
{
	const float SafeMax = FMath::Max(MaxHealth, KINDA_SMALL_NUMBER);
	SetHealthPercent(CurrentHealth / SafeMax);
}

FText UMeidoHUD::LabelForMeiDouInput(const EMeiDouInput Input)
{
	switch (Input)
	{
	case EMeiDouInput::EMDI_Moe:
		return FText::FromString(TEXT("M"));
	case EMeiDouInput::EMDI_Kyun:
		return FText::FromString(TEXT("K"));
	case EMeiDouInput::EMDI_Nyan:
		return FText::FromString(TEXT("N"));
	default:
		return FText::GetEmpty();
	}
}

void UMeidoHUD::SetSlotText(UTextBlock* const SlotWidget, const EMeiDouInput Input, const bool bFilled)
{
	if (!SlotWidget)
	{
		return;
	}

	if (bFilled && Input != EMeiDouInput::None)
	{
		SlotWidget->SetText(LabelForMeiDouInput(Input));
		SlotWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
	else
	{
		SlotWidget->SetText(FText::GetEmpty());
		SlotWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}
