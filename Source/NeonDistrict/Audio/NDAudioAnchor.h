// Copyright Neon District Sandbox. Public benchmark repo — original content only.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NDAudioAnchor.generated.h"

class UNDSynthAudioComponent;

/**
 * Owns the procedural synth component for a level. Spawned by the world
 * subsystem alongside the district builder (and in the menu for menu music).
 * Registers itself with the GameInstance AudioManager so playback calls
 * (ambience, engine, siren, blips) always reach a live synth.
 */
UCLASS()
class NEONDISTRICT_API ANDAudioAnchor : public AActor
{
	GENERATED_BODY()

public:
	ANDAudioAnchor();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintPure, Category = "Audio")
	UNDSynthAudioComponent* GetSynth() const { return Synth; }

private:
	UPROPERTY(VisibleAnywhere, Category = "Audio")
	TObjectPtr<UNDSynthAudioComponent> Synth = nullptr;
};
