// Copyright Neon District Sandbox. Public benchmark repo — original content only.

#include "UI/NDMainMenuWidget.h"
#include "Core/NDGameInstance.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void UNDMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UWidgetTree* Tree = WidgetTree;
	if (!Tree)
	{
		return;
	}

	UCanvasPanel* Root = Tree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("ND_MenuRoot"));
	Tree->RootWidget = Root;

	// Title.
	UTextBlock* Title = Tree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ND_Title"));
	Title->SetText(FText::FromString(TEXT("NEON DISTRICT")));
	Title->SetFont(FSlateFontInfo(nullptr, 64, FName("Bold")));
	Title->SetColorAndOpacity(FLinearColor(1.0f, 0.1f, 0.6f, 1.0f));
	UCanvasPanelSlot* TitleSlot = Root->AddChildToCanvas(Title);
	TitleSlot->SetAnchors(FAnchors(0.5f, 0.35f, 0.5f, 0.35f));
	TitleSlot->SetAlignment(FVector2D(0.5f, 0.5f));
	TitleSlot->SetAutoSize(true);

	UTextBlock* Subtitle = Tree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ND_Subtitle"));
	Subtitle->SetText(FText::FromString(TEXT("SANDBOX")));
	Subtitle->SetFont(FSlateFontInfo(nullptr, 28, FName("Bold")));
	Subtitle->SetColorAndOpacity(FLinearColor(0.1f, 0.95f, 1.0f, 1.0f));
	UCanvasPanelSlot* SubSlot = Root->AddChildToCanvas(Subtitle);
	SubSlot->SetAnchors(FAnchors(0.5f, 0.42f, 0.5f, 0.42f));
	SubSlot->SetAlignment(FVector2D(0.5f, 0.5f));
	SubSlot->SetAutoSize(true);

	// Buttons.
	ButtonBox = Tree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("ND_ButtonBox"));
	UCanvasPanelSlot* BoxSlot = Root->AddChildToCanvas(ButtonBox);
	BoxSlot->SetAnchors(FAnchors(0.5f, 0.55f, 0.5f, 0.55f));
	BoxSlot->SetAlignment(FVector2D(0.5f, 0.5f));
	BoxSlot->SetAutoSize(true);

	UButton* PlayBtn = MakeButton(FText::FromString(TEXT("JUGAR")));
	PlayBtn->OnClicked.AddDynamic(this, &UNDMainMenuWidget::OnPlayClicked);

	UButton* ContinueBtn = MakeButton(FText::FromString(TEXT("CONTINUAR")));
	ContinueBtn->OnClicked.AddDynamic(this, &UNDMainMenuWidget::OnContinueClicked);

	UButton* QuitBtn = MakeButton(FText::FromString(TEXT("SALIR")));
	QuitBtn->OnClicked.AddDynamic(this, &UNDMainMenuWidget::OnQuitClicked);

	// Controls hint.
	UTextBlock* Hint = Tree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ND_Hint"));
	Hint->SetText(FText::FromString(
		TEXT("WASD mover · Ratón cámara · Shift correr · Espacio saltar/freno\nE interactuar · F entrar/salir vehículo · ESC pausa · F5 guardar · F9 cargar")));
	Hint->SetFont(FSlateFontInfo(nullptr, 14));
	Hint->SetColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.7f, 1.0f));
	UCanvasPanelSlot* HintSlot = Root->AddChildToCanvas(Hint);
	HintSlot->SetAnchors(FAnchors(0.5f, 0.9f, 0.5f, 0.9f));
	HintSlot->SetAlignment(FVector2D(0.5f, 0.5f));
	HintSlot->SetAutoSize(true);
}

UButton* UNDMainMenuWidget::MakeButton(const FText& Label)
{
	UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
	Button->SetMinDesiredWidth(320.0f);

	UTextBlock* ButtonLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	ButtonLabel->SetText(Label);
	ButtonLabel->SetFont(FSlateFontInfo(nullptr, 22, FName("Bold")));
	ButtonLabel->SetColorAndOpacity(FLinearColor(0.05f, 0.05f, 0.1f, 1.0f));
	Button->AddChild(ButtonLabel);

	ButtonBox->AddChildToVerticalBox(Button);
	return Button;
}

void UNDMainMenuWidget::OnPlayClicked()
{
	UGameplayStatics::OpenLevel(this, FName("ND_City"));
}

void UNDMainMenuWidget::OnContinueClicked()
{
	if (UNDGameInstance* GI = GetGameInstance<UNDGameInstance>())
	{
		GI->LoadGame();
	}
	UGameplayStatics::OpenLevel(this, FName("ND_City"));
}

void UNDMainMenuWidget::OnQuitClicked()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}
