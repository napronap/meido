#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MeidoTutorialWidget.generated.h"

class UTextBlock;
class UImage;
class UButton;
class AMeidoDemoGameMode;

/** WBP supplies the layout. Native bindings handle slides and mouse buttons. */
UCLASS(Abstract, Blueprintable)
class MEIDO_5_6_API UMeidoTutorialWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void RefreshSlide();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> SlideTitleText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> SlideBodyText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> SlidePageText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UImage> SlideImage;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> NextSlideButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> PreviousSlideButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> NextSlideLabel;

private:
	UPROPERTY(Transient)
	TObjectPtr<AMeidoDemoGameMode> BoundTutorialFlow;

	UFUNCTION()
	void HandleSlideChanged(int32 InSlideIndex, int32 InSlideCount);

	UFUNCTION()
	void HandleNextClicked();

	UFUNCTION()
	void HandlePreviousClicked();
};
