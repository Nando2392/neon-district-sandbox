// Copyright Neon District Sandbox. Public benchmark repo — original content only.

#include "Core/NDGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Audio/NDAudioManager.h"
#include "FX/NDVFXManager.h"

void UNDGameInstance::Init()
{
	Super::Init();

	// Audio manager is a plain UObject owned here (no asset dependency to bootstrap).
	AudioManager = NewObject<UNDAudioManager>(this, TEXT("NDAudioManager"));
	AudioManager->InitializeFromSave(GetMutableSave());
	AudioManager->ApplyVolumes();

	VFXManager = NewObject<UNDVFXManager>(this, TEXT("NDVFXManager"));
}

void UNDGameInstance::Shutdown()
{
	if (AudioManager)
	{
		AudioManager->ShutdownAudio();
		AudioManager = nullptr;
	}
	VFXManager = nullptr;
	Super::Shutdown();
}

bool UNDGameInstance::SaveGame()
{
	if (!CurrentSave)
	{
		CurrentSave = NewObject<UNDSaveGame>(this);
	}

	// Pull live state from the world when one exists.
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0))
		{
			if (APawn* Pawn = PC->GetPawn())
			{
				CurrentSave->PlayerLocation = Pawn->GetActorLocation();
				CurrentSave->PlayerYaw = Pawn->GetActorRotation().Yaw;
			}
		}
	}

	bHasLoadedOrSaved = UGameplayStatics::SaveGameToSlot(CurrentSave, SlotName, UserIndex);
	return bHasLoadedOrSaved;
}

bool UNDGameInstance::LoadGame()
{
	if (USaveGame* Loaded = UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex))
	{
		CurrentSave = Cast<UNDSaveGame>(Loaded);
		if (CurrentSave)
		{
			bHasLoadedOrSaved = true;
			if (AudioManager)
			{
				AudioManager->InitializeFromSave(CurrentSave);
				AudioManager->ApplyVolumes();
			}
			return true;
		}
	}

	// No save yet: start a fresh snapshot.
	CurrentSave = NewObject<UNDSaveGame>(this);
	return false;
}

void UNDGameInstance::SetMasterVolume(float Volume)
{
	if (!CurrentSave)
	{
		CurrentSave = NewObject<UNDSaveGame>(this);
	}
	CurrentSave->MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
	if (AudioManager)
	{
		AudioManager->ApplyVolumes();
	}
}
