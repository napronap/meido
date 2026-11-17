#include "UI/MeidoTutorialWidget.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/World.h"
#include "GameModes/MeidoDemoGameMode.h"
#include "PlayerControllers/DemoMaidPlayerController.h"

void UMeidoTutorialWidget::NativeConstruct()
{
	Super::NativeConstruct();
	BoundTutorialFlow = GetWorld() ? GetWorld()->GetAuthGameMode<AMeidoDemoGameMode>() : nullptr;
	if (BoundTutorialFlow)
	{
		BoundTutorialFlow->OnTutorialSlideChanged.AddUniqueDynamic(this, &UMeidoTutorialWidget::HandleSlideChanged);
	}
	if (NextSlideButton)
	{
		NextSlideButton->OnClicked.AddUniqueDynamic(this, &UMeidoTutorialWidget::HandleNextClicked);
	}
	if (PreviousSlideButton)
	{
		PreviousSlideButton->OnClicked.AddUniqueDynamic(this, &UMeidoTutorialWidget::HandlePreviousClicked);
	}
	RefreshSlide();
}

void UMeidoTutorialWidget::NativeDestruct()
{
	if (IsValid(BoundTutorialFlow))
	{
		BoundTutorialFlow->OnTutorialSlideChanged.RemoveDynamic(this, &UMeidoTutorialWidget::HandleSlideChanged);
	}
	BoundTutorialFlow = nullptr;
	if (NextSlideButton)
	{
		NextSlideButton->OnClicked.RemoveDynamic(this, &UMeidoTutorialWidget::HandleNextClicked);
	}
	if (PreviousSlideButton)
	{
		PreviousSlideButton->OnClicked.RemoveDynamic(this, &UMeidoTutorialWidget::HandlePreviousClicked);
	}
	Super::NativeDestruct();
}

void UMeidoTutorialWidget::RefreshSlide()
{
	FMeidoTutorialSlide CurrentSlide;
	if (!IsValid(BoundTutorialFlow) || !BoundTutorialFlow->GetCurrentTutorialSlide(CurrentSlide))
	{
		return;
	}
	const int32 CurrentSlideIndex = BoundTutorialFlow->GetTutorialSlideIndex();
	const int32 TotalSlideCount = BoundTutorialFlow->GetTutorialSlideCount();
	if (SlideTitleText)
	{
		SlideTitleText->SetText(CurrentSlide.Title);
	}
	if (SlideBodyText)
	{
		SlideBodyText->SetText(CurrentSlide.Body);
	}
	if (SlidePageText)
	{
		SlidePageText->SetText(FText::Format(NSLOCTEXT("MeidoDemo", "SlidePage", "{0} / {1}"),
			FText::AsNumber(CurrentSlideIndex + 1), FText::AsNumber(TotalSlideCount)));
	}
	if (SlideImage)
	{
		SlideImage->SetBrushFromTexture(CurrentSlide.Image.Get());
		SlideImage->SetVisibility(CurrentSlide.Image ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}
	if (PreviousSlideButton)
	{
		PreviousSlideButton->SetIsEnabled(CurrentSlideIndex > 0);
	}
	if (NextSlideLabel)
	{
		NextSlideLabel->SetText(CurrentSlideIndex + 1 < TotalSlideCount
			? NSLOCTEXT("MeidoDemo", "NextSlide", "Next") : NSLOCTEXT("MeidoDemo", "StartPlaying", "Start"));
	}
}

void UMeidoTutorialWidget::HandleSlideChanged(int32 InSlideIndex, int32 InSlideCount)
{
	RefreshSlide();
}

void UMeidoTutorialWidget::HandleNextClicked()
{
	if (ADemoMaidPlayerController* TutorialController = Cast<ADemoMaidPlayerController>(GetOwningPlayer()))
	{
		TutorialController->AdvanceTutorial();
	}
}

void UMeidoTutorialWidget::HandlePreviousClicked()
{
	if (ADemoMaidPlayerController* TutorialController = Cast<ADemoMaidPlayerController>(GetOwningPlayer()))
	{
		TutorialController->PreviousTutorialSlide();
	}
}
