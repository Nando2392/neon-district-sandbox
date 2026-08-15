// Copyright Neon District Sandbox. Public benchmark repo — original content only.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "NDGameMode.generated.h"

/**
 * Default game mode: wires the player character, HUD class and input setup.
 * City population (NPCs, traffic, pickups) is spawned by level logic actors,
 * never from here — keeps spawn ownership explicit and capped (NDPerf).
 */
UCLASS()
class NEONDISTRICT_API ANDGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ANDGameMode();

	virtual void BeginPlay() override;

	/** HUD widget class (UMG). Assigned in editor via Content/UI assets or left null for code-created HUD. */
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> HUDWidgetClass;
};
