// Copyright Neon District Sandbox. Public benchmark repo — original content only.

#include "UI/NDHUDWidget.h"
#include "Systems/NDWantedSystem.h"
#include "Systems/NDMissionSystem.h"

#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Blueprint/WidgetTree.h"

void UNDHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Subscribing to events only (HUD never polls game state).
	if (UNDWantedSystem* Wanted = GetGameInstance()->GetSubsystem<UNDWantedSystem>())
	{
		Wanted->OnWantedLevelChanged.AddDynamic(this, &UNDHUDWidget::HandleWantedChanged);
		HandleWantedChanged(Wanted->GetWantedLevel());
	}
	if (NDMissionSystem* Mission = GetGameInstance()->GetSubsystem<NDMissionSystem>())
	{
		Mission->OnMissionStageChanged.AddDynamic(this, &UNDHUDWidget::HandleMissionChanged);
		HandleMissionChanged(Mission->GetMissionStage());
	}

	BuildProceduralHUD();
}

void UNDHUDWidget::BuildProceduralHUD()
{
	if (RootCanvas)
	{
		return; // already built (or an editor BP provided a layout)
	}

	UWidgetTree* Tree = WidgetTree;
	if (!Tree)
	{
		return;
	}

	RootCanvas = Tree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("ND_RootCanvas"));
	Tree->RootWidget = RootCanvas;

	// --- Objective + wanted (top-left) ---
	UBorder* InfoBorder = Tree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("ND_InfoBorder"));
	InfoBorder->SetBrushColor(FLinearColor(0.02f, 0.02f, 0.06f, 0.55f));
	UCanvasPanelSlot* InfoSlot = RootCanvas->AddChildToCanvas(InfoBorder);
	InfoSlot->SetAnchors(FAnchors(0.0f, 0.0f, 0.0f, 0.0f));
	InfoSlot->SetAlignment(FVector2D(0.0f, 0.0f));
	InfoSlot->SetPosition(FVector2D(24.0f, 20.0f));
	InfoSlot->SetSize(FVector2D(560.0f, 110.0f));

	UVerticalBox* InfoBox = Tree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("ND_InfoBox"));
	InfoBorder->SetContent(InfoBox);

	ObjectiveTextBlock = Tree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ND_Objective"));
	ObjectiveTextBlock->SetFont(FSlateFontInfo(nullptr, 22, FName("Bold")));
	ObjectiveTextBlock->SetColorAndOpacity(FLinearColor(0.25f, 0.95f, 1.0f, 1.0f));
	InfoBox->AddChildToVerticalBox(ObjectiveTextBlock);

	WantedTextBlock = Tree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ND_Wanted"));
	WantedTextBlock->SetFont(FSlateFontInfo(nullptr, 18, FName("Bold")));
	InfoBox->AddChildToVerticalBox(WantedTextBlock);

	// --- Interaction prompt (bottom-center) ---
	PromptTextBlock = Tree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ND_Prompt"));
	PromptTextBlock->SetFont(FSlateFontInfo(nullptr, 24, FName("Bold")));
	PromptTextBlock->SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 0.9f, 1.0f));
	UCanvasPanelSlot* PromptSlot = RootCanvas->AddChildToCanvas(PromptTextBlock);
	PromptSlot->SetAnchors(FAnchors(0.5f, 1.0f, 0.5f, 1.0f));
	PromptSlot->SetAlignment(FVector2D(0.5f, 1.0f));
	PromptSlot->SetPosition(FVector2D(0.0f, -60.0f));
	PromptSlot->SetAutoSize(true);

	// --- Notification (center) ---
	NotificationBorder = Tree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("ND_NotificationBorder"));
	NotificationBorder->SetBrushColor(FLinearColor(0.05f, 0.05f, 0.1f, 0.75f));
	NotificationBorder->SetVisibility(ESlateVisibility::Hidden);
	UCanvasPanelSlot* NotifSlot = RootCanvas->AddChildToCanvas(NotificationBorder);
	NotifSlot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
	NotifSlot->SetAlignment(FVector2D(0.5f, 0.5f));
	NotifSlot->SetPosition(FVector2D(0.0f, -160.0f));
	NotifSlot->SetAutoSize(true);

	NotificationTextBlock = Tree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ND_Notification"));
	NotificationTextBlock->SetFont(FSlateFontInfo(nullptr, 20));
	NotificationTextBlock->SetColorAndOpacity(FLinearColor(1.0f, 0.9f, 0.6f, 1.0f));
	UBorder* NotifContent = NotificationBorder;
	NotifContent->SetContent(NotificationTextBlock);
}

void UNDHUDWidget::SetObjectiveText(const FText& Text)
{
	if (ObjectiveTextBlock)
	{
		ObjectiveTextBlock->SetText(Text);
	}
	OnObjectiveChanged(Text);
}

void UNDHUDWidget::SetWantedLevel(int32 Level)
{
	if (WantedTextBlock)
	{
		FLinearColor Color = FLinearColor::White;
		FString Label = TEXT("Sin buscado");
		if (Level == 1) { Label = TEXT("NIVEL 1 — busca discreta"); Color = FLinearColor(1.0f, 0.9f, 0.2f); }
		else if (Level == 2) { Label = TEXT("NIVEL 2 — refuerzos en camino"); Color = FLinearColor(1.0f, 0.5f, 0.1f); }
		else if (Level >= 3) { Label = TEXT("NIVEL 3 — ¡huida total!"); Color = FLinearColor(1.0f, 0.1f, 0.1f); }
		WantedTextBlock->SetText(FText::FromString(Label));
		WantedTextBlock->SetColorAndOpacity(Color);
	}
	OnWantedChanged(Level);
}

void UNDHUDWidget::SetInteractionPrompt(const FText& Text)
{
	if (PromptTextBlock)
	{
		PromptTextBlock->SetText(Text);
		PromptTextBlock->SetVisibility(Text.IsEmpty() ? ESlateVisibility::Hidden : ESlateVisibility::Visible);
	}
	OnInteractionPromptChanged(Text);
}

void UNDHUDWidget::SetVehicleState(bool bInVehicleIn, const FText& VehicleName)
{
	bInVehicle = bInVehicleIn;
	if (bInVehicle && ObjectiveTextBlock)
	{
		ObjectiveTextBlock->SetText(FText::Format(
			FText::FromString(TEXT("Conduciendo: {0}   |   W acelerar · S frenar · A/D girar · Espacio freno de mano · F salir")),
			VehicleName));
	}
	OnVehicleStateChanged(bInVehicleIn, VehicleName);
}

void UNDHUDWidget::ShowNotification(const FText& Text)
{
	if (NotificationBorder && NotificationTextBlock)
	{
		NotificationTextBlock->SetText(Text);
		NotificationBorder->SetVisibility(ESlateVisibility::Visible);
		NotificationTimer = 4.0f;
	}
	OnNotification(Text);
}

void UNDHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	TickNotification(InDeltaTime);
}

void UNDHUDWidget::TickNotification(float DeltaSeconds)
{
	if (NotificationTimer <= 0.0f)
	{
		return;
	}
	NotificationTimer -= DeltaSeconds;
	if (NotificationTimer <= 0.0f && NotificationBorder)
	{
		NotificationBorder->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UNDHUDWidget::HandleWantedChanged(int32 NewLevel)
{
	SetWantedLevel(NewLevel);
}

void UNDHUDWidget::HandleMissionChanged(int32 NewStage)
{
	if (NDMissionSystem* Mission = GetGameInstance()->GetSubsystem<NDMissionSystem>())
	{
		SetObjectiveText(Mission->GetObjectiveText());
	}
}
