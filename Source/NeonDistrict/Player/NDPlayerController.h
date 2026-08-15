// Copyright Neon District Sandbox. Public benchmark repo — original content only

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "NDPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class ANDCharacter;
class ANDVehicle;
class UUserWidget;
class AActor;

class UNDHUDWidget;

/**
 * Player controller: Enhanced Input fallback system with classical input bindings,
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

	UFUNCTION(BlueprintPure, Category = "HUD")
	UNDHUDWidget* GetHUDWidget() const;

	/** Toggle pause + pause widget (Escape). */
	void HandlePause();

	/** Resume from the pause widget. */
	void HandlePauseFromWidget();

	// Public for benchmark testing
	void TestMoveForward(float Value);
	void TestMoveRight(float Value);
	void TestJump();
	void TestInteract();

private:
	void SetupGameplayInput();

	// --- Enhanced Input (created at runtime so the repo needs zero input assets) ---
	void HandleMove(const struct FInputActionValue& Value);
	void HandleLook(const struct FInputActionValue& Value);
	void HandleJumpStart();
	void HandleJumpStop();
	void HandleSprintStart();
	void HandleSprintStop();
	void HandleInteract();
	void HandleEnterExitVehicle();
	void HandleQuickSave();
	void HandleQuickLoad();

	// --- Fallback classical Input (for builds without Enhanced Input assets) ---
	void SetupFallbackInput();
	void HandleMoveForwardFallback(float AxisValue);
	void HandleMoveRightFallback(float AxisValue);
	void HandleLookHorizontalFallback(float AxisValue);
	void HandleLookVerticalFallback(float AxisValue);
	void HandleMoveAxis(const FVector& Direction, float Value);
	bool TrySetupEnhancedInput();
	void BindEnhancedInput(class UEnhancedInputComponent* EIC);
	void SetupInputMappings();

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
	TObjectPtr<UUserWidget> PauseWidget = nullptr;

	UPROPERTY()
	TObjectPtr<ANDVehicle> DrivenVehicle = nullptr;

	/** True if using fallback classical InputComponent bindings */
	bool bUsingFallback = false;

	bool bIsDriving = false;
};