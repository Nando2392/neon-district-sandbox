// Copyright Neon District Sandbox. Public benchmark repo — original content only.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NDBenchmarkRunner.generated.h"

class ANDVehicle;
class ANDNPCCharacter;
class ANDPlayerController;

/**
 * Headless benchmark driver (no GUI interaction needed).
 *
 * Activated with `-benchmark` on the command line. Runs phased checks with
 * timers so the world has time to build and populate, takes real screenshots
 * (including forced gameplay states for the visual gate), writes a PASS/FAIL
 * report to Saved/Benchmark/NDBenchmarkResult.txt and exits with code 0
 * (all pass) or 1 (any fail). Evidence is real engine output: actor counts,
 * subsystem state, pause round-trip, movement, save/load, screenshot files.
 *
 * On ND_MainMenu it only captures the menu screenshot (the visual gate's
 * main_menu.png) and exits PASS — gameplay phases require the city.
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
	/** Timer-driven phases: 0 world, 1 actors, 2 mission/wanted, 3 vehicle, 4 controls, 5 save/load, 6 screenshots, 7 report+exit. */
	void RunPhase();
	void PhaseWorldReady();
	void PhaseActors();
	void PhaseMissionAndWanted();
	void PhaseVehicle();
	void PhaseWeapons();
	void PhaseControls();
	void PhaseSaveLoad();
	void PhaseScreenshots();
	void PhaseFinish();
	void PhaseMenuOnly();

	/** Movement check sub-flow (drives the pawn forward a few ticks). */
	void TickMoveCheck();

	void Check(bool bPass, const FString& Gate, const FString& Detail);
	void Screenshot(const FString& Name);

	/** Point the player camera at an actor and park the pawn near it. */
	void FrameTarget(AActor* Target);
	/** Exterior three-quarter vehicle shot that does not let the pawn cover the car. */
	void FrameVehicleShowcase(AActor* Target);
	/** Deterministic street-level shot aimed at a populated district block. */
	void FrameWorldShowcase();

	/** Find the mission NPC with the given role (giver=Mei, delivery=Nova). */
	ANDNPCCharacter* FindMissionNPC(int32 RoleIndex) const;

	UPROPERTY()
	TObjectPtr<AActor> Spawner = nullptr;

	FTimerHandle PhaseTimerHandle;
	FTimerHandle MoveTimerHandle;
	int32 PhaseIndex = 0;
	int32 PassCount = 0;
	int32 FailCount = 0;
	bool bReportWritten = false;
	bool bInMenu = false;

	FVector MoveStartLoc = FVector::ZeroVector;
	int32 MoveTicksLeft = 0;
	int32 MoveStage = 0;
	int32 MoveSettleTicks = 0;

	TArray<FString> ReportLines;
};
