// Copyright Neon District Sandbox. Public benchmark repo — original content only.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "NDSaveGame.h"
#include "NDGameInstance.generated.h"

class UNDAudioManager;
class UNDVFXManager;
class APlayerController;

/**
 * Owns save/load, audio manager and cross-map flow (menu <-> city).
 * Mission/wanted state lives in GameInstance subsystems, so it survives
 * level transitions without serialization gymnastics.
 */
UCLASS()
class NEONDISTRICT_API UNDGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	/** Save current progress to the default slot (atomic temp+rename handled by UE SaveGameToSlot). */
	UFUNCTION(BlueprintCallable, Category = "Save")
	bool SaveGame();

	/** Load a save; returns false when no valid save exists. */
	UFUNCTION(BlueprintCallable, Category = "Save")
	bool LoadGame();

	UFUNCTION(BlueprintPure, Category = "Save")
	bool HasSave() const { return bHasLoadedOrSaved; }

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void SetMasterVolume(float Volume);

	UFUNCTION(BlueprintPure, Category = "Audio")
	float GetMasterVolume() const { return CurrentSave ? CurrentSave->MasterVolume : 0.85f; }

	/** Called by systems to keep the snapshot fresh before writing. */
	UNDSaveGame* GetMutableSave() { return CurrentSave; }

	/** Called by NDAudioManager after bootstrapping. */
	void RegisterAudioManager(UNDAudioManager* Manager) { AudioManager = Manager; }
	UNDAudioManager* GetAudioManager() const { return AudioManager; }

	/** Niagara FX manager (owned here; spawn is event-driven from gameplay code). */
	UNDVFXManager* GetVFXManager() const { return VFXManager; }

	/** Convenience for PlayerController startup. */
	void CachePendingPlayer(APlayerController* PC) { PendingPlayer = PC; }
	APlayerController* TakePendingPlayer() { APlayerController* PC = PendingPlayer; PendingPlayer = nullptr; return PC; }

	virtual void Init() override;
	virtual void Shutdown() override;

private:
	static constexpr const TCHAR* SlotName = TEXT("NeonDistrictSlot");
	static constexpr int32 UserIndex = 0;

	UPROPERTY()
	TObjectPtr<UNDSaveGame> CurrentSave = nullptr;

	UPROPERTY()
	TObjectPtr<UNDAudioManager> AudioManager = nullptr;

	UPROPERTY()
	TObjectPtr<UNDVFXManager> VFXManager = nullptr;

	UPROPERTY()
	TObjectPtr<APlayerController> PendingPlayer = nullptr;

	bool bHasLoadedOrSaved = false;
};
