// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/MeiDouPoseBubbleWidget.h"

#include "Components/TextBlock.h"

void UMeiDouPoseBubbleWidget::ShowPose(const EMeiDouInput Input)
{
	const FText Display = TextForInput(Input);
	ShowCustomText(Display);
	OnBubbleShown(Input, Display);
}

void UMeiDouPoseBubbleWidget::ShowCustomText(const FText& Text)
{
	if (UTextBlock* const Block = PoseText.Get())
	{
		Block->SetText(Text);
	}
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

void UMeiDouPoseBubbleWidget::HideBubble()
{
	OnBubbleHidden();
	SetVisibility(ESlateVisibility::Collapsed);
}

void UMeiDouPoseBubbleWidget::OnBubbleShown_Implementation(EMeiDouInput /*Input*/, const FText& /*DisplayText*/)
{
	// BP: appear anim / scale punch
}

void UMeiDouPoseBubbleWidget::OnBubbleHidden_Implementation()
{
	// BP: optional cleanup
}

FText UMeiDouPoseBubbleWidget::TextForInput(const EMeiDouInput Input) const
{
	switch (Input)
	{
	case EMeiDouInput::EMDI_Moe:
		return MoeText.IsEmpty() ? FText::FromString(TEXT("萌え～")) : MoeText;
	case EMeiDouInput::EMDI_Kyun:
		return KyunText.IsEmpty() ? FText::FromString(TEXT("キュン")) : KyunText;
	case EMeiDouInput::EMDI_Nyan:
		return NyanText.IsEmpty() ? FText::FromString(TEXT("ニャン")) : NyanText;
	default:
		return FText::GetEmpty();
	}
}
