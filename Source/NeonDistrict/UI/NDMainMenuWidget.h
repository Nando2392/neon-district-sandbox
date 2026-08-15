// Copyright Neon District Sandbox. Public benchmark repo — original content only.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NDMainMenuWidget.generated.h"

class UButton;
class UTextBlock;
class UVerticalBox;

/**
 * Main menu (procedural UMG, zero assets): title + Play / Continue / Quit.
 * Game mode opens ND_City; Continue first tries to load a save.
 */
UCLASS()
class NEONDISTRICT_API UNDMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

private:
	UFUNCTION()
	void OnPlayClicked();

	UFUNCTION()
	void OnContinueClicked();

	UFUNCTION()
	void OnQuitClicked();

	UButton* MakeButton(const FText& Label);

	UPROPERTY()
	TObjectPtr<UVerticalBox> ButtonBox = nullptr;
};
