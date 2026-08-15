// Copyright Neon District Sandbox. Public benchmark repo — original content only.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "NDAudioManager.generated.h"

class USoundBase;
class UAudioComponent;

/**
 * Audio manager (owned by the GameInstance). Bus layout by category with
 * per-category volume multipliers; assets are optional soft paths configured in
 * the editor — with zero assets the game stays silent but never crashes.
 * Wanted intensity drives ambience/music layering + siren.
 */
UCLASS()
class NEONDISTRICT_API UNDAudioManager : public UObject
{
	GENERATED_BODY()

public:
	void InitializeFromSave(UNDSaveGame* Save);
	void ApplyVolumes();

	/** Called by the player controller on wanted level changes. */
	void SetWantedLevel(int32 Level);

	void PlayFootstep(AActor* Context);
	void PlayImpact(AActor* Context);
	void PlayUI();
	void PlayAlert();
	void StartEngineLoop(AActor* Vehicle);
	void UpdateEngineSound(AActor* Vehicle, float SpeedKmh);
	void StopEngineLoop();
	void ShutdownAudio();

	// Editor-configurable assets (soft paths, may be empty).
	UPROPERTY(EditAnywhere, Category = "Audio|Assets")
	TArray<TSoftObjectPtr<USoundBase>> FootstepSounds;

	UPROPERTY(EditAnywhere, Category = "Audio|Assets")
	TSoftObjectPtr<USoundBase> EngineLoopSound;

	UPROPERTY(EditAnywhere, Category = "Audio|Assets")
	TSoftObjectPtr<USoundBase> SirenSound;

	UPROPERTY(EditAnywhere, Category = "Audio|Assets")
	TSoftObjectPtr<USoundBase> ImpactSound;

	UPROPERTY(EditAnywhere, Category = "Audio|Assets")
	TSoftObjectPtr<USoundBase> UIClickSound;

	UPROPERTY(EditAnywhere, Category = "Audio|Assets")
	TSoftObjectPtr<USoundBase> AlertSound;

	// Category volumes (1.0 = full).
	UPROPERTY(EditAnywhere, Category = "Audio|Volumes")
	float MasterVolume = 0.85f;

	UPROPERTY(EditAnywhere, Category = "Audio|Volumes")
	float MusicVolume = 0.70f;

	UPROPERTY(EditAnywhere, Category = "Audio|Volumes")
	float SfxVolume = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Audio|Volumes")
	float AmbienceVolume = 0.6f;

	UPROPERTY(EditAnywhere, Category = "Audio|Volumes")
	float UiVolume = 0.8f;

	UPROPERTY(EditAnywhere, Category = "Audio|Volumes")
	float VehiclesVolume = 0.8f;

	UPROPERTY(EditAnywhere, Category = "Audio|Volumes")
	float DialogueVolume = 0.9f;

private:
	void PlayOneShot(const TSoftObjectPtr<USoundBase>& Sound, AActor* Context, float CategoryVolume, float PitchJitter = 0.0f);
	void UpdateSiren(AActor* Context);

	UPROPERTY()
	TObjectPtr<UAudioComponent> EngineComponent = nullptr;

	UPROPERTY()
	TObjectPtr<UAudioComponent> SirenComponent = nullptr;

	UPROPERTY()
	TObjectPtr<AActor> EngineContext = nullptr;

	int32 WantedLevel = 0;
	float EnginePitch = 1.0f;
	bool bSirenActive = false;
};
