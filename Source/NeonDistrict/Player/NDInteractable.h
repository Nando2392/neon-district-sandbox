// Copyright Neon District Sandbox. Public benchmark repo — original content only.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "NDInteractable.generated.h"

/**
 * Contract for anything the player can interact with (NPCs, vehicles, pickups).
 * Implementers return a prompt string and handle the interaction.
 */
UINTERFACE(MinimalAPI, Blueprintable)
class UNDIInteractable : public UInterface
{
	GENERATED_BODY()
};

class NEONDISTRICT_API INDIInteractable
{
	GENERATED_BODY()

public:
	/** Prompt shown in the HUD, e.g. "Hablar con Mei (E)". Empty = nothing to do. */
	UFUNCTION(BlueprintNativeEvent, Category = "Interaction")
	FText GetInteractionPrompt() const;

	/** Player pressed interact on this target. Returns true if the interaction consumed the press. */
	UFUNCTION(BlueprintNativeEvent, Category = "Interaction")
	bool Interact(APlayerController* Instigator);
};
