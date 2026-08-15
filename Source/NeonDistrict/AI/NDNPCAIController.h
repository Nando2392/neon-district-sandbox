// Copyright Neon District Sandbox. Public benchmark repo — original content only.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "NDNPCAIController.generated.h"

class APlayerController;
class ACharacter;

UENUM()
enum class ENPNPCBehavior : uint8
{
	PatrolCivilian UMETA(DisplayName = "Patrol Civilian"),
	FleeCivilian   UMETA(DisplayName = "Flee Civilian"),
	ChasePlayer    UMETA(DisplayName = "Chase Player"),
	SearchForPlayer UMETA(DisplayName = "Search For Player"),
	ReturnToPatrol  UMETA(DisplayName = "Return To Patrol")
};

/**
 * Navmesh-driven NPC controller with a small FSM (no BT asset required):
 *   Civilian: patrol waypoints; flee briefly when wanted heat is active near them.
 *   Police:   detect (distance + line of sight) -> chase -> lose sight timer -> return.
 * Reports detection/evasion to the Wanted system so heat is shared across units.
 */
UCLASS()
class NEONDISTRICT_API ANDNPCAIController : public AAIController
{
	GENERATED_BODY()

public:
	ANDNPCAIController();

	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaSeconds) override;

	void SetPatrolPoints(const TArray<FVector>& Points, bool bIsPolice);

	UFUNCTION(BlueprintPure, Category = "NPC")
	bool IsPolice() const { return bPolice; }

private:
	void UpdatePatrol(float DeltaSeconds);
	void UpdatePolice(float DeltaSeconds);
	void UpdateFlee(float DeltaSeconds);
	bool CanSeePlayer(ACharacter* Player, FVector& OutViewPoint) const;

	TArray<FVector> PatrolPoints;
	int32 NextPatrolIndex = 0;
	float PatrolWaitTimer = 0.0f;
	float LoseSightTimer = 0.0f;
	float FleeTimer = 0.0f;
	bool bPolice = false;
	ENPNPCBehavior Behavior = ENPNPCBehavior::PatrolCivilian;

	// Mission-giver NPCs keep patrolling tight loops; police hold position.
	static constexpr float PatrolWaitSeconds = 1.8f;
	static constexpr float FleeSeconds = 3.5f;
};
