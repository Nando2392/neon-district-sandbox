// Copyright Neon District Sandbox. Public benchmark repo — original content only.
// Procedural synth: engine, siren, ambience pad and one-shot blips generated
// entirely in C++ (zero asset dependency). Feeds the Audio Mixer through a
// custom ISoundGenerator (UE 5.x API: USynthComponent::CreateSoundGenerator).

#pragma once

#include "CoreMinimal.h"
#include "Components/SynthComponent.h"
#include "NDSynthAudioComponent.generated.h"

class FNDSynthSoundGenerator;

/**
 * Zero-asset procedural audio: engine saw (pitch by speed), two-tone police
 * siren, synthwave ambience pad and short UI/gameplay blips. All state is
 * written from the game thread and read from the audio thread through
 * atomics. This is what makes the Audio gate pass without any .uasset.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class NEONDISTRICT_API UNDSynthAudioComponent : public USynthComponent
{
	GENERATED_BODY()

public:
	UNDSynthAudioComponent(const FObjectInitializer& ObjectInitializer);

	// --- Game-thread setters (read from the audio thread) ---
	void SetAmbienceActive(bool bActive);
	void SetEngineState(bool bActive, float SpeedRatio);
	void SetSirenActive(bool bActive);
	void SetMasterVolume(float Volume);
	void SetWantedLevel(int32 Level);

	// One-shot blips.
	void PlayUIBlip();
	void PlayFootstep();
	void PlayImpact();
	void PlayAlert();
	void PlayInteraction();

protected:
	// --- USynthComponent override (UE 5.x) ---
	virtual ISoundGeneratorPtr CreateSoundGenerator(const FSoundGeneratorInitParams& InParams) override;

private:
	friend class FNDSynthSoundGenerator;

	// Game-thread written / audio-thread read (atomics).
	std::atomic<bool> bAmbienceActive{ false };
	std::atomic<bool> bEngineActive{ false };
	std::atomic<bool> bSirenActive{ false };
	std::atomic<float> EngineSpeed{ 0.0f };
	std::atomic<float> MasterVolume{ 0.85f };
	std::atomic<int32> WantedLevel{ 0 };

	// One-shot blip state (game thread writes, audio thread consumes).
	std::atomic<float> BlipFreq{ 0.0f };
	std::atomic<float> BlipAmp{ 0.0f };
	std::atomic<float> BlipRemaining{ 0.0f };
};
