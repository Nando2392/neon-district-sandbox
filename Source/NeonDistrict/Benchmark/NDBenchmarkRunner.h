// Copyright Neon District Sandbox. Public benchmark repo — original content only.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NDBenchmarkRunner.generated.h"

/**
 * Headless PIE benchmark driver (no GUI interaction needed).
 *
 * Activated with `-benchmark` on the command line. Runs phased checks with
 * timers so the world has time to build and populate, takes real screenshots,
 * writes a PASS/FAIL report to Saved/Benchmark/NDBenchmarkResult.txt and exits
 * with code 0 (all pass) or 1 (any fail). Evidence is real engine output:
 * actor counts, subsystem state, save/load round-trip, screenshot files.
 */
UCLASS()
class NEONDISTRICT_API ANDBenchmarkRunner : public AActor
{
	GENERATED_BODY()

public:
	ANDBenchmarkRunner();

	virtual void BeginPlay() override;

	/** True when the `-benchmark` command line flag is present. */
	static bool IsBenchmarkMode();

private:
	/** Timer-driven phases: 0 world, 1 actors, 2 mission/wanted, 3 vehicle, 4 save/load, 5 report+exit. */
	void RunPhase();
	void PhaseWorldReady();
	void PhaseActors();
	void PhaseMissionAndWanted();
	void PhaseVehicle();
	void PhaseSaveLoad();
	void PhaseScreenshots();
	void PhaseFinish();

	void Check(bool bPass, const FString& Gate, const FString& Detail);
	void Screenshot(const FString& Name);
	void WriteReport();

	UPROPERTY()
	TObjectPtr<AActor> Spawner = nullptr;

	FTimerHandle PhaseTimerHandle;
	int32 PhaseIndex = 0;
	int32 PassCount = 0;
	int32 FailCount = 0;
	bool bReportWritten = false;

	TArray<FString> ReportLines;
};
