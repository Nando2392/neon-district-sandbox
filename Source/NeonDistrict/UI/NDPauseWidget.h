// Copyright Neon District Sandbox. Public benchmark repo — original content only.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NDPauseWidget.generated.h"

class UBorder;
class UTextBlock;

/**
 * Pause overlay: blocks the game (world paused by the controller), shows
 * controls, resumes on Escape, opens the main menu on M. Visual-only; all
 * pause state lives in the player controller / world pause.
 */
UCLASS()
class NEONDISTRICT_API UNDPauseWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

private:
	void BuildLayout();

	UPROPERTY()
	TObjectPtr<UBorder> RootBorder = nullptr;
	UPROPERTY()
	TObjectPtr<UTextBlock> TitleText = nullptr;
	UPROPERTY()
	TObjectPtr<UTextBlock> HintText = nullptr;
};
