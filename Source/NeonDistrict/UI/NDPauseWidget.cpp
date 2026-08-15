// Copyright Neon District Sandbox. Public benchmark repo — original content only.

#include "UI/NDPauseWidget.h"
#include "Player/NDPlayerController.h"

#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Blueprint/WidgetTree.h"
#include "Kismet/GameplayStatics.h"
#include "Input/Events.h"

void UNDPauseWidget::NativeConstruct()
{
	Super::NativeConstruct();
	BuildLayout();
}

FReply UNDPauseWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();
	if (Key == EKeys::Escape)
	{
		if (ANDPlayerController* PC = Cast<ANDPlayerController>(GetOwningPlayer()))
		{
			PC->HandlePauseFromWidget(); // resume
		}
		return FReply::Handled();
	}
	if (Key == EKeys::M)
	{
		UGameplayStatics::OpenLevel(GetWorld(), FName("ND_MainMenu"));
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UNDPauseWidget::BuildLayout()
{
	UWidgetTree* Tree = WidgetTree;
	if (!Tree)
	{
		return;
	}

	RootBorder = Tree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("ND_PauseRoot"));
	RootBorder->SetBrushColor(FLinearColor(0.01f, 0.01f, 0.05f, 0.82f));
	Tree->RootWidget = RootBorder;

	UVerticalBox* Box = Tree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("ND_PauseBox"));
	RootBorder->SetContent(Box);

	TitleText = Tree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ND_PauseTitle"));
	TitleText->SetText(FText::FromString(TEXT("PAUSA")));
	TitleText->SetFont(FSlateFontInfo(nullptr, 44, FName("Bold")));
	TitleText->SetColorAndOpacity(FLinearColor(0.35f, 0.9f, 1.0f, 1.0f));
	Box->AddChildToVerticalBox(TitleText);

	HintText = Tree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ND_PauseHint"));
	HintText->SetText(FText::FromString(
		TEXT("ESC — reanudar\nM — menú principal\nF5 — guardar   F9 — cargar")));
	HintText->SetFont(FSlateFontInfo(nullptr, 20));
	HintText->SetColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.95f, 1.0f));
	Box->AddChildToVerticalBox(HintText);
}
