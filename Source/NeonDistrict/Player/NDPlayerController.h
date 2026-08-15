// Copyright Neon District Sandbox. Public benchmark repo — original content only.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "NDPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class ANDCharacter;
class ANDVehicle;
class UNDHUDWidget;
class UNDPauseWidget;
class AActor;

/**
 * Player controller: Enhanced Input (runtime-created, no asset dependency),
 * interaction raycast from the camera, vehicle enter/exit, pause/menu, quick save/load.
 */
UCLASS()
class NEONDISTRICT_API ANDPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ANDPlayerController();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupInputComponent() override;

	/** Called by NDVehicle when the player enters/exits. */
	void SetDrivingState(ANDVehicle* Vehicle, bool bEntering);

	UFUNCTION(BlueprintPure, Category = "Player")
	bool IsDriving() const { return bIsDriving; }

	UFUNCTION(BlueprintPure, Category = "Player")
	ANDVehicle* GetDrivenVehicle() const { return DrivenVehicle; }

	ANDCharacter* GetPlayerCharacter() const { return PlayerCharacter; }

	UFUNCTION(BlueprintPure, Category = "Player")
	UNDHUDWidget* GetHUDWidget() const { return HUDWidget; }

	/** Resume from the pause widget (also bound to Escape in-game). */
	void HandlePauseFromWidget();

private:
	// --- Enhanced Input (created at runtime so the repo needs zero input assets) ---
	void CreateInputActions();
	void HandleMove(const struct FInputActionValue& Value);
	void HandleLook(const struct FInputActionValue& Value);
	void HandleJumpStart();
	void HandleJumpStop();
	void HandleSprintStart();
	void HandleSprintStop();
	void HandleInteract();
	void HandleEnterExitVehicle();
	void HandlePause();
	void HandleQuickSave();
	void HandleQuickLoad();

	// --- Interaction ---
	void UpdateInteractionTarget();
	void RestoreCharacterFromVehicle();

	UPROPERTY()
	TObjectPtr<UInputMappingContext> InputContext = nullptr;

	UPROPERTY()
	TObjectPtr<UInputAction> IA_Move = nullptr;
	UPROPERTY()
	TObjectPtr<UInputAction> IA_Look = nullptr;
	UPROPERTY()
	TObjectPtr<UInputAction> IA_Jump = nullptr;
	UPROPERTY()
	TObjectPtr<UInputAction> IA_Sprint = nullptr;
	UPROPERTY()
	TObjectPtr<UInputAction> IA_Interact = nullptr;
	UPROPERTY()
	TObjectPtr<UInputAction> IA_Vehicle = nullptr;
	UPROPERTY()
	TObjectPtr<UInputAction> IA_Pause = nullptr;
	UPROPERTY()
	TObjectPtr<UInputAction> IA_QuickSave = nullptr;
	UPROPERTY()
	TObjectPtr<UInputAction> IA_QuickLoad = nullptr;

	UPROPERTY()
	TObjectPtr<ANDCharacter> PlayerCharacter = nullptr;

	UPROPERTY()
	TObjectPtr<AActor> CurrentInteractable = nullptr;

	UPROPERTY()
	TObjectPtr<UNDHUDWidget> HUDWidget = nullptr;

	UPROPERTY()
	TObjectPtr<UNDPauseWidget> PauseWidget = nullptr;

	UPROPERTY()
	TObjectPtr<ANDVehicle> DrivenVehicle = nullptr;

	bool bIsDriving = false;
};
