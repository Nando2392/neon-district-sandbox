// Copyright Neon District Sandbox. Public benchmark repo — original content only.

#include "Audio/NDSynthAudioComponent.h"

#include "Math/UnrealMathUtility.h"
#include "Sound/SoundGenerator.h"

// ---------------------------------------------------------------------------
// Sound generator: runs on the audio thread, reads the component's atomics.
// ---------------------------------------------------------------------------
class FNDSynthSoundGenerator : public ISoundGenerator
{
public:
	FNDSynthSoundGenerator(UNDSynthAudioComponent* InOwner, int32 InSampleRate)
		: Owner(InOwner)
		, SampleRate(InSampleRate > 0 ? InSampleRate : 48000)
	{
	}

	virtual int32 OnGenerateAudio(float* OutAudio, int32 NumSamples) override
	{
		const float DeltaTime = 1.0f / static_cast<float>(SampleRate);
		for (int32 i = 0; i < NumSamples; ++i)
		{
			OutAudio[i] = GenerateSample(TimeSec);
			TimeSec += DeltaTime;
		}
		return NumSamples;
	}

	virtual int32 GetDesiredNumSamplesToRenderPerCallback() const override
	{
		return 1024;
	}

	virtual void OnEndGenerate() override
	{
		Owner = nullptr;
	}

private:
	float GenerateSample(double T)
	{
		if (!Owner)
		{
			return 0.0f;
		}

		const float Master = Owner->MasterVolume.load();

		// --- Ambience pad: soft synthwave chord (A minor-ish) with slow LFO ---
		float Ambience = 0.0f;
		if (Owner->bAmbienceActive.load())
		{
			const double Lfo = 0.5 + 0.5 * FMath::Sin(2.0 * PI * 0.15 * T);
			const float Chord[3] = { 110.0f, 164.81f, 220.0f }; // A2, E3, A3
			for (int32 n = 0; n < 3; ++n)
			{
				const double Phase = 2.0 * PI * static_cast<double>(Chord[n]) * T;
				Ambience += SoftSaw(Phase, 0.5f) * (0.035f + 0.02f * static_cast<float>(Lfo));
			}
			// Very slow breath.
			Ambience *= static_cast<float>(0.65 + 0.35 * FMath::Sin(2.0 * PI * 0.05 * T));
		}

		// --- Engine: pitch follows speed ratio, volume too ---
		float Engine = 0.0f;
		if (Owner->bEngineActive.load())
		{
			const float Speed = Owner->EngineSpeed.load();
			const float BaseFreq = 46.0f + Speed * 68.0f; // ~46Hz idle -> ~114Hz flat out
			const double Phase = 2.0 * PI * static_cast<double>(BaseFreq) * T;
			const float Sub = FMath::Sin(Phase * 0.5);
			Engine = SoftSaw(Phase, 0.6f) * (0.16f + 0.14f * Speed)
				+ Sub * (0.12f + 0.10f * Speed)
				+ Noise(T, 7.3) * 0.02f * (0.3f + Speed);
		}

		// --- Siren: two-tone police when wanted >= 2 ---
		float Siren = 0.0f;
		if (Owner->bSirenActive.load())
		{
			const double Cycle = FMath::Fmod(T, 0.55);
			const float Freq = Cycle < 0.275 ? 640.0f : 880.0f;
			const double Phase = 2.0 * PI * static_cast<double>(Freq) * T;
			Siren = FMath::Sin(Phase) * 0.13f;
		}

		// --- One-shot blips (UI, footstep, impact, alert, interaction) ---
		float Blip = 0.0f;
		const float BlipLeft = Owner->BlipRemaining.load();
		if (BlipLeft > 0.0f)
		{
			const float Freq = Owner->BlipFreq.load();
			const float Amp = Owner->BlipAmp.load();
			const float Env = FMath::Exp(-14.0f * BlipLeft);
			Blip = FMath::Sin(2.0 * PI * static_cast<double>(Freq) * T) * Amp * Env;
			Owner->BlipRemaining.store(BlipLeft - 1.0f / static_cast<float>(SampleRate));
		}

		float Out = Ambience + Engine + Siren + Blip;
		Out = FMath::Clamp(Out, -1.0f, 1.0f) * Master;
		return Out;
	}

	float SoftSaw(double Phase, float Rolloff) const
	{
		// Sum of a few harmonics with rolloff approximates a band-limited saw.
		float Sum = 0.0f;
		for (int32 h = 1; h <= 6; ++h)
		{
			Sum += FMath::Sin(static_cast<double>(h) * Phase) * FMath::Pow(Rolloff, static_cast<float>(h - 1)) / static_cast<float>(h);
		}
		return Sum * 0.66f;
	}

	float Noise(double T, float Seed) const
	{
		// Deterministic pseudo-noise: cheap hash of time + seed, audio-thread safe.
		const double V = FMath::Sin(T * 12.9898 + Seed * 78.233) * 43758.5453;
		return static_cast<float>(V - FMath::Floor(V)) * 2.0f - 1.0f;
	}

	UNDSynthAudioComponent* Owner = nullptr;
	int32 SampleRate = 48000;
	double TimeSec = 0.0;
};

// ---------------------------------------------------------------------------
// Component
// ---------------------------------------------------------------------------
UNDSynthAudioComponent::UNDSynthAudioComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Stereo output; the generator fills both channels with the same sample.
	NumChannels = 2;
	PrimaryComponentTick.bCanEverTick = false;
}

void UNDSynthAudioComponent::SetAmbienceActive(bool bActive)
{
	bAmbienceActive.store(bActive);
}

void UNDSynthAudioComponent::SetEngineState(bool bActive, float SpeedRatio)
{
	bEngineActive.store(bActive);
	EngineSpeed.store(FMath::Clamp(SpeedRatio, 0.0f, 1.0f));
}

void UNDSynthAudioComponent::SetSirenActive(bool bActive)
{
	bSirenActive.store(bActive);
}

void UNDSynthAudioComponent::SetMasterVolume(float Volume)
{
	MasterVolume.store(FMath::Clamp(Volume, 0.0f, 1.0f));
}

void UNDSynthAudioComponent::SetWantedLevel(int32 Level)
{
	WantedLevel.store(FMath::Max(0, Level));
	SetSirenActive(Level >= 2);
}

void UNDSynthAudioComponent::PlayUIBlip()
{
	BlipFreq.store(880.0f);
	BlipAmp.store(0.22f);
	BlipRemaining.store(0.09f);
}

void UNDSynthAudioComponent::PlayFootstep()
{
	BlipFreq.store(140.0f);
	BlipAmp.store(0.16f);
	BlipRemaining.store(0.05f);
}

void UNDSynthAudioComponent::PlayImpact()
{
	BlipFreq.store(95.0f);
	BlipAmp.store(0.30f);
	BlipRemaining.store(0.12f);
}

void UNDSynthAudioComponent::PlayAlert()
{
	// Rising two-note alert.
	BlipFreq.store(660.0f);
	BlipAmp.store(0.28f);
	BlipRemaining.store(0.22f);
}

void UNDSynthAudioComponent::PlayInteraction()
{
	BlipFreq.store(520.0f);
	BlipAmp.store(0.24f);
	BlipRemaining.store(0.10f);
}

ISoundGeneratorPtr UNDSynthAudioComponent::CreateSoundGenerator(const FSoundGeneratorInitParams& InParams)
{
	return MakeShared<FNDSynthSoundGenerator, ESPMode::ThreadSafe>(this, InParams.SampleRate);
}
