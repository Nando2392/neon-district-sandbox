// Copyright Neon District Sandbox. Public benchmark repo — original content only.

#include "Audio/NDAudioManager.h"
#include "Core/NDSaveGame.h"

#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Engine/World.h"
#include "UObject/SoftObjectPtr.h"

void UNDAudioManager::InitializeFromSave(UNDSaveGame* Save)
{
	if (Save)
	{
		MasterVolume = Save->MasterVolume;
		MusicVolume = Save->MusicVolume;
		SfxVolume = Save->SfxVolume;
	}
}

void UNDAudioManager::ApplyVolumes()
{
	// Without USoundClass assets the per-category volumes are applied at call sites
	// via PlayOneShot; editor can wire real submixes into the sounds for bus control.
	if (EngineComponent)
	{
		EngineComponent->SetVolumeMultiplier(VehiclesVolume * MasterVolume);
	}
	if (SirenComponent)
	{
		SirenComponent->SetVolumeMultiplier(VehiclesVolume * MasterVolume);
	}
}

void UNDAudioManager::SetWantedLevel(int32 Level)
{
	WantedLevel = Level;
	// Siren + music intensity only when actively pursued (level 2+).
	if (Level >= 2 && EngineContext)
	{
		if (!bSirenActive)
		{
			bSirenActive = true;
			UpdateSiren(EngineContext);
		}
	}
	else if (bSirenActive)
	{
		bSirenActive = false;
		if (SirenComponent)
		{
			SirenComponent->Stop();
		}
	}
}

void UNDAudioManager::PlayOneShot(const TSoftObjectPtr<USoundBase>& Sound, AActor* Context, float CategoryVolume, float PitchJitter)
{
	if (Sound.IsNull())
	{
		return; // asset not configured yet: stay silent, never crash
	}
	USoundBase* Loaded = Sound.LoadSynchronous();
	if (!Loaded)
	{
		return;
	}
	const float Pitch = 1.0f + PitchJitter * FMath::FRandRange(-1.0f, 1.0f);
	UGameplayStatics::PlaySound2D(Context, Loaded, MasterVolume * CategoryVolume, Pitch);
}

void UNDAudioManager::PlayFootstep(AActor* Context)
{
	if (FootstepSounds.Num() == 0)
	{
		return;
	}
	const int32 Index = FMath::RandRange(0, FootstepSounds.Num() - 1);
	PlayOneShot(FootstepSounds[Index], Context, SfxVolume * 0.6f, 0.08f);
}

void UNDAudioManager::PlayImpact(AActor* Context)
{
	PlayOneShot(ImpactSound, Context, SfxVolume * 0.9f, 0.1f);
}

void UNDAudioManager::PlayUI()
{
	PlayOneShot(UIClickSound, nullptr, UiVolume);
}

void UNDAudioManager::PlayAlert()
{
	PlayOneShot(AlertSound, nullptr, SfxVolume * 0.9f);
}

void UNDAudioManager::StartEngineLoop(AActor* Vehicle)
{
	EngineContext = Vehicle;
	if (EngineLoopSound.IsNull())
	{
		return;
	}

	USoundBase* Loaded = EngineLoopSound.LoadSynchronous();
	if (!Loaded || !Vehicle)
	{
		return;
	}

	if (!EngineComponent)
	{
		EngineComponent = NewObject<UAudioComponent>(Vehicle);
		EngineComponent->bAutoActivate = false;
		EngineComponent->bIsUISound = false;
		EngineComponent->RegisterComponent();
		EngineComponent->AttachToComponent(Vehicle->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	}
	EngineComponent->SetSound(Loaded);
	EngineComponent->SetVolumeMultiplier(VehiclesVolume * MasterVolume);
	EngineComponent->SetPitchMultiplier(0.7f);
	EngineComponent->Play();
}

void UNDAudioManager::UpdateEngineSound(AActor* Vehicle, float SpeedKmh)
{
	if (!EngineComponent)
	{
		return;
	}
	// RPM-ish pitch by speed; tapers so idle is quiet and full throttle is loud.
	const float SpeedRatio = FMath::Clamp(SpeedKmh / 130.0f, 0.0f, 1.0f);
	EnginePitch = FMath::Lerp(0.7f, 1.45f, SpeedRatio);
	EngineComponent->SetPitchMultiplier(EnginePitch);
	EngineComponent->SetVolumeMultiplier((0.25f + 0.75f * SpeedRatio) * VehiclesVolume * MasterVolume);

	// Siren engages when wanted level is high enough while driving.
	if (WantedLevel >= 2 && !bSirenActive)
	{
		bSirenActive = true;
		UpdateSiren(Vehicle);
	}
}

void UNDAudioManager::UpdateSiren(AActor* Context)
{
	if (SirenSound.IsNull() || !Context)
	{
		return;
	}
	USoundBase* Loaded = SirenSound.LoadSynchronous();
	if (!Loaded)
	{
		return;
	}
	if (!SirenComponent)
	{
		SirenComponent = NewObject<UAudioComponent>(Context);
		SirenComponent->bAutoActivate = false;
		SirenComponent->bIsUISound = false;
		SirenComponent->RegisterComponent();
		SirenComponent->AttachToComponent(Context->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	}
	SirenComponent->SetSound(Loaded);
	SirenComponent->SetVolumeMultiplier(VehiclesVolume * MasterVolume);
	SirenComponent->Play();
}

void UNDAudioManager::StopEngineLoop()
{
	if (EngineComponent)
	{
		EngineComponent->Stop();
	}
	if (SirenComponent)
	{
		SirenComponent->Stop();
		bSirenActive = false;
	}
	EngineContext = nullptr;
}

void UNDAudioManager::ShutdownAudio()
{
	StopEngineLoop();
	if (EngineComponent)
	{
		EngineComponent->DestroyComponent();
		EngineComponent = nullptr;
	}
	if (SirenComponent)
	{
		SirenComponent->DestroyComponent();
		SirenComponent = nullptr;
	}
}
