// Copyright Neon District Sandbox. Public benchmark repo — original content only.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NDCitySpawner.generated.h"

class ANDNPCCharacter;
class ANDVehicle;
class ANDTrafficVehicle;
class USplineComponent;

/**
 * City population spawner: caps are explicit (NDPerf), spawn happens once at
 * BeginPlay — never from Tick. Places civilians, police, drivable vehicles and
 * spline traffic routes, then hands each NPC its patrol assignment.
 */
UCLASS()
class NEONDISTRICT_API ANDCitySpawner : public AActor
{
	GENERATED_BODY()

public:
	ANDCitySpawner();

	virtual void BeginPlay() override;

	/** District radius used to pick spawn points (should cover the 2 blocks). */
	UPROPERTY(EditAnywhere, Category = "Spawner")
	float DistrictRadius = 1800.0f;

	UPROPERTY(EditAnywhere, Category = "Spawner")
	int32 CivilianCount = 12;

	UPROPERTY(EditAnywhere, Category = "Spawner")
	int32 PoliceCount = 2;

	UPROPERTY(EditAnywhere, Category = "Spawner")
	int32 DrivableVehicleCount = 3;

	UPROPERTY(EditAnywhere, Category = "Spawner")
	int32 TrafficVehicleCount = 3;

	UPROPERTY(EditAnywhere, Category = "Spawner")
	TSubclassOf<ANDNPCCharacter> CivilianClass;

	UPROPERTY(EditAnywhere, Category = "Spawner")
	TSubclassOf<ANDNPCCharacter> PoliceClass;

	UPROPERTY(EditAnywhere, Category = "Spawner")
	TSubclassOf<ANDVehicle> VehicleClass;

	UPROPERTY(EditAnywhere, Category = "Spawner")
	TSubclassOf<ANDTrafficVehicle> TrafficClass;

	/** Mission NPCs: index 0 = giver (Mei), 1 = package, 2 = delivery (Nova). */
	UPROPERTY(EditAnywhere, Category = "Spawner|Mission")
	TArray<TSubclassOf<ANDNPCCharacter>> MissionNPCClasses;

private:
	FVector PickNavSpawnPoint(float Radius) const;
	void SpawnCivilians();
	void SpawnPolice();
	void SpawnVehicles();
	void SpawnTraffic();
	TArray<USplineComponent*> CollectTrafficRoutes() const;
};
