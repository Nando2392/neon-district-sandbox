// Copyright Neon District Sandbox. Public benchmark repo — original content only.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "NDWantedSystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNDOnWantedLevelChanged, int32, NewLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNDOnHeatEvent, float, Intensity);

/**
 * Wanted/heat system — 3 levels, timer-based decay, event-driven (HUD + audio subscribe).
 * Level 1: light pursuit. Level 2: more units, larger radius. Level 3: aggressive + siren.
 */
UCLASS()
class NEONDISTRICT_API UNDWantedSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintPure, Category = "Wanted")
	int32 GetWantedLevel() const { return WantedLevel; }

	/** Called by police AI when the player is detected (raises heat toward next level). */
	UFUNCTION(BlueprintCallable, Category = "Wanted")
	void ReportDetection(float Intensity = 1.0f);

	/** Called by police AI when the player is no longer detected. */
	UFUNCTION(BlueprintCallable, Category = "Wanted")
	void ReportEvasion();

	UFUNCTION(BlueprintCallable, Category = "Wanted")
	void SetWantedLevel(int32 NewLevel);

	UFUNCTION(BlueprintCallable, Category = "Wanted")
	void ClearWanted();

	UPROPERTY(BlueprintAssignable, Category = "Wanted")
	FNDOnWantedLevelChanged OnWantedLevelChanged;

	UPROPERTY(BlueprintAssignable, Category = "Wanted")
	FNDOnHeatEvent OnHeatEvent;

private:
	void TickDecay();
	void ApplyLevel(int32 NewLevel);

	FTimerHandle DecayTimerHandle;
	float HeatAccumulator = 0.0f;
	int32 WantedLevel = 0;
	bool bEvading = true;

	static constexpr float HeatPerDetection = 34.0f;
	static constexpr float Level1Threshold = 34.0f;   // first sighting -> level 1
	static constexpr float Level2Threshold = 70.0f;   // sustained sighting / ramming
	static constexpr float Level3Threshold = 100.0f;  // police ram / long chase
};
