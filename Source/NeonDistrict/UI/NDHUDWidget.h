// Copyright Neon District Sandbox. Public benchmark repo — original content only.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NDHUDWidget.generated.h"

class UCanvasPanel;
class UTextBlock;
class UBorder;

/**
 * HUD: objective, wanted level, interaction prompt, vehicle state, notifications.
 * Builds a procedural UMG layout at runtime (zero asset dependency) and exposes
 * BlueprintImplementableEvents so an editor-authored Widget Blueprint can replace
 * the visuals while keeping the same bindings.
 */
UCLASS()
class NEONDISTRICT_API UNDHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void SetObjectiveText(const FText& Text);

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void SetWantedLevel(int32 Level);

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void SetInteractionPrompt(const FText& Text);

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void SetVehicleState(bool bVehicleActive, const FText& VehicleName);

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void SetWeaponState(bool bEquipped, int32 Ammo);

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void ShowNotification(const FText& Text);

	// Editor-overridable presentation hooks (called after state changes).
	UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
	void OnObjectiveChanged(const FText& Text);

	UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
	void OnWantedChanged(int32 Level);

	UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
	void OnInteractionPromptChanged(const FText& Text);

	UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
	void OnVehicleStateChanged(bool bVehicleActive, const FText& VehicleName);

	UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
	void OnNotification(const FText& Text);

private:
	UFUNCTION()
	void HandleWantedChanged(int32 NewLevel);

	UFUNCTION()
	void HandleMissionChanged(int32 NewStage);

	void BuildProceduralHUD();
	void TickNotification(float DeltaSeconds);

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY()
	TObjectPtr<UCanvasPanel> RootCanvas = nullptr;
	UPROPERTY()
	TObjectPtr<UTextBlock> ObjectiveTextBlock = nullptr;
	UPROPERTY()
	TObjectPtr<UTextBlock> WantedTextBlock = nullptr;
	UPROPERTY()
	TObjectPtr<UTextBlock> PromptTextBlock = nullptr;
	UPROPERTY()
	TObjectPtr<UTextBlock> WeaponTextBlock = nullptr;
	UPROPERTY()
	TObjectPtr<UTextBlock> NotificationTextBlock = nullptr;
	UPROPERTY()
	TObjectPtr<UBorder> NotificationBorder = nullptr;

	float NotificationTimer = 0.0f;
	bool bInVehicle = false;
};
