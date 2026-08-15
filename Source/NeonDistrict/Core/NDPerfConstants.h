// Copyright Neon District Sandbox. Public benchmark repo — original content only.

#pragma once

#include "CoreMinimal.h"

/**
 * Single home for every tuning/limit constant (performance/config contract).
 * Explicit caps so nothing spawns unbounded; no spawning actors from Tick anywhere.
 */
namespace NDPerf
{
	// --- Caps ---
	inline constexpr int32 MaxCivilianNPCs = 14;      // explicit NPC cap (benchmark: >=10 walking)
	inline constexpr int32 MaxPoliceNPCs = 3;         // wanted escalation units
	inline constexpr int32 MaxTrafficVehicles = 4;    // parked + patrol traffic
	inline constexpr int32 MaxDriveableVehicles = 3;  // benchmark: >=3 vehicles, 1 drivable
	inline constexpr int32 MaxFXActors = 24;          // pooled Niagara spawn ceiling

	// --- Movement (cm/s) ---
	inline constexpr float WalkSpeed = 250.0f;
	inline constexpr float RunSpeed = 600.0f;
	inline constexpr float SprintSpeed = 850.0f;
	inline constexpr float JumpVelocity = 520.0f;

	// --- Camera ---
	inline constexpr float CameraSpringArmLength = 360.0f;
	inline constexpr float CameraMinLength = 120.0f;
	inline constexpr float CameraMaxLength = 700.0f;
	inline constexpr float CameraPitchMin = -50.0f;
	inline constexpr float CameraPitchMax = 25.0f;
	inline constexpr float CameraCollisionProbeSize = 14.0f;

	// --- Wanted / heat ---
	inline constexpr int32 MaxWantedLevel = 3;
	inline constexpr float WantedDecayDelay = 6.0f;   // seconds without detection before decay starts
	inline constexpr float WantedDecayInterval = 2.5f; // seconds per level drop
	inline constexpr float PoliceDetectionRadius = 1400.0f;
	inline constexpr float PoliceChaseSpeed = 700.0f;
	inline constexpr float PoliceLoseRadius = 3200.0f; // beyond this, cops lose the player
	inline constexpr float PoliceLoseDelay = 4.0f;     // visible-line lost before giving up

	// --- Interaction ---
	inline constexpr float InteractionRange = 260.0f;
	inline constexpr float InteractionProbeRadius = 60.0f;

	// --- Mission ---
	inline constexpr float MissionMarkerAltitude = 140.0f; // HUD marker float height
}
