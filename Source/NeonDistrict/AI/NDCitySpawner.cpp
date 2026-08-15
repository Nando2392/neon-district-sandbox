// Copyright Neon District Sandbox. Public benchmark repo — original content only.

#include "AI/NDCitySpawner.h"
#include "AI/NDNPCCharacter.h"
#include "AI/NDNPCAIController.h"
#include "Vehicle/NDVehicle.h"
#include "Vehicle/NDTrafficVehicle.h"
#include "Core/NDPerfConstants.h"
#include "Systems/NDMissionSystem.h"

#include "NavigationSystem.h"
#include "Components/SplineComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

ANDCitySpawner::ANDCitySpawner()
{
	PrimaryActorTick.bCanEverTick = false;

	// --- Default classes for zero-asset spawning ---
	// Civilian: basic character with procedural mesh.
	static ConstructorHelpers::FClassFinder<ANDNPCCharacter> CivilianBP(TEXT("/Script/Engine.Class'/Game/Blueprints/BP_Civilian.BP_Civilian_C'"));
	if (!CivilianBP.Succeeded())
	{
		CivilianClass = ANDNPCCharacter::StaticClass();
	}
	else
	{
		CivilianClass = CivilianBP.Class;
	}

	// Police: same character class with police flag.
	static ConstructorHelpers::FClassFinder<ANDNPCCharacter> PoliceBP(TEXT("/Script/Engine.Class'/Game/Blueprints/BP_Police.BP_Police_C'"));
	if (!PoliceBP.Succeeded())
	{
		PoliceClass = ANDNPCCharacter::StaticClass();
	}
	else
	{
		PoliceClass = PoliceBP.Class;
	}

	// Vehicle: drivable car.
	static ConstructorHelpers::FClassFinder<ANDVehicle> VehicleBP(TEXT("/Script/Engine.Class'/Game/Blueprints/BP_CityCar.BP_CityCar_C'"));
	if (!VehicleBP.Succeeded())
	{
		VehicleClass = ANDVehicle::StaticClass();
	}
	else
	{
		VehicleClass = VehicleBP.Class;
	}

	// Traffic vehicle: autonomous.
	static ConstructorHelpers::FClassFinder<ANDTrafficVehicle> TrafficBP(TEXT("/Script/Engine.Class'/Game/Blueprints/BP_TrafficCar.BP_TrafficCar_C'"));
	if (!TrafficBP.Succeeded())
	{
		TrafficClass = ANDTrafficVehicle::StaticClass();
	}
	else
	{
		TrafficClass = TrafficBP.Class;
	}

	// Mission NPCs: Mei (giver), Package holder (meetup), Nova (delivery).
	// Using the same NPC class; different roles handled at spawn time.
	MissionNPCClasses.Add(ANDNPCCharacter::StaticClass());
	MissionNPCClasses.Add(ANDNPCCharacter::StaticClass());
	MissionNPCClasses.Add(ANDNPCCharacter::StaticClass());
}

void ANDCitySpawner::BeginPlay()
{
	Super::BeginPlay();

	// Caps from NDPerf: never spawn more than the budget.
	CivilianCount = FMath::Min(CivilianCount, NDPerf::MaxCivilianNPCs);
	PoliceCount = FMath::Min(PoliceCount, NDPerf::MaxPoliceNPCs);
	DrivableVehicleCount = FMath::Min(DrivableVehicleCount, NDPerf::MaxDriveableVehicles);
	TrafficVehicleCount = FMath::Min(TrafficVehicleCount, NDPerf::MaxTrafficVehicles);

	SpawnCivilians();
	SpawnPolice();
	SpawnVehicles();
	SpawnTraffic();
	SpawnMissionNPCs();
}

FVector ANDCitySpawner::PickNavSpawnPoint(float Radius) const
{
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	FVector Location = GetActorLocation();
	FNavLocation NavLocation;
	if (NavSys && NavSys->GetRandomReachablePointInRadius(GetActorLocation(), Radius, NavLocation))
	{
		Location = NavLocation;
	}
	return Location;
}

void ANDCitySpawner::SpawnCivilians()
{
	if (!CivilianClass)
	{
		return;
	}

	const TArray<FString> Names = { TEXT("Mei"), TEXT("Jon"), TEXT("Aria"), TEXT("Kai"),
		TEXT("Sasha"), TEXT("Dex"), TEXT("Lena"), TEXT("Otto"), TEXT("Iris"), TEXT("Marco"),
		TEXT("Nia"), TEXT("Theo"), TEXT("Rex"), TEXT("Vera") };

	for (int32 i = 0; i < CivilianCount; ++i)
	{
		const FVector SpawnPoint = PickNavSpawnPoint(DistrictRadius);
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		if (ANDNPCCharacter* NPC = GetWorld()->SpawnActor<ANDNPCCharacter>(CivilianClass, SpawnPoint, FRotator::ZeroRotator, Params))
		{
			NPC->ConfigureNPC(false, Names[i % Names.Num()], ENPCMissionRole::None, i % 4);
			NPC->PatrolPoints = { SpawnPoint + FVector(FMath::FRandRange(-600.0f, 600.0f), FMath::FRandRange(-600.0f, 600.0f), 0.0f),
				SpawnPoint + FVector(FMath::FRandRange(-400.0f, 400.0f), FMath::FRandRange(-400.0f, 400.0f), 0.0f) };
			if (ANDNPCAIController* AIC = Cast<ANDNPCAIController>(NPC->GetController()))
			{
				AIC->SetPatrolPoints(NPC->PatrolPoints, false);
			}
		}
	}
}

void ANDCitySpawner::SpawnPolice()
{
	if (!PoliceClass)
	{
		return;
	}

	const TArray<FString> Names = { TEXT("Oficial Nox"), TEXT("Oficial Vega"), TEXT("Oficial Rook") };
	for (int32 i = 0; i < PoliceCount; ++i)
	{
		const FVector SpawnPoint = PickNavSpawnPoint(DistrictRadius * 0.8f);
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		if (ANDNPCCharacter* NPC = GetWorld()->SpawnActor<ANDNPCCharacter>(PoliceClass, SpawnPoint, FRotator::ZeroRotator, Params))
		{
			NPC->ConfigureNPC(true, Names[i % Names.Num()], ENPCMissionRole::None, 4 + i);
			NPC->PatrolPoints = { SpawnPoint, SpawnPoint + FVector(0.0f, 300.0f, 0.0f) };
			if (ANDNPCAIController* AIC = Cast<ANDNPCAIController>(NPC->GetController()))
			{
				AIC->SetPatrolPoints(NPC->PatrolPoints, true);
			}
		}
	}
}

void ANDCitySpawner::SpawnVehicles()
{
	if (!VehicleClass)
	{
		return;
	}

	for (int32 i = 0; i < DrivableVehicleCount; ++i)
	{
		const FVector SpawnPoint = PickNavSpawnPoint(DistrictRadius);
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		GetWorld()->SpawnActor<ANDVehicle>(VehicleClass, SpawnPoint, FRotator(0.0f, FMath::FRandRange(-180.0f, 180.0f), 0.0f), Params);
	}
}

void ANDCitySpawner::SpawnTraffic()
{
	if (!TrafficClass)
	{
		return;
	}

	const TArray<USplineComponent*> Routes = CollectTrafficRoutes();
	if (Routes.Num() == 0)
	{
		return;
	}

	for (int32 i = 0; i < TrafficVehicleCount; ++i)
	{
		USplineComponent* Route = Routes[i % Routes.Num()];
		const float Distance = Route->GetSplineLength() * FMath::FRandRange(0.0f, 0.9f);
		const FVector SpawnPoint = Route->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
		const FRotator SpawnRotation = Route->GetRotationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		if (ANDTrafficVehicle* Traffic = GetWorld()->SpawnActor<ANDTrafficVehicle>(TrafficClass, SpawnPoint, SpawnRotation, Params))
		{
			Traffic->SetRoute(Route);
		}
	}
}

void ANDCitySpawner::SpawnMissionNPCs()
{
	// Roles: index 0 = Mei (MissionGiver), index 1 = Package, index 2 = Nova (Delivery)
	static constexpr ENPCMissionRole MissionRoles[] = {
		ENPCMissionRole::MissionGiver,
		ENPCMissionRole::Package,
		ENPCMissionRole::Delivery
	};

	static constexpr const TCHAR* MissionNames[] = {
		TEXT("Mei"),
		TEXT("Nova"),
		TEXT("Paquete")
	};

	if (MissionNPCClasses.Num() == 0)
	{
		return;
	}

	for (int32 i = 0; i < MissionNPCClasses.Num(); ++i)
	{
		const FVector SpawnPoint = PickNavSpawnPoint(DistrictRadius * 0.6f);
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		if (ANDNPCCharacter* NPC = GetWorld()->SpawnActor<ANDNPCCharacter>(MissionNPCClasses[i], SpawnPoint, FRotator::ZeroRotator, Params))
		{
			// Mei stands at a specific location in the northern avenue.
			FVector NPCSpawn = SpawnPoint;
			if (i == 0) // Mei (giver)
			{
				NPCSpawn = PickNavSpawnPoint(DistrictRadius * 0.5f) + FVector(0, -DistrictRadius * 0.5f, 0);
				NPC->PatrolPoints = { NPCSpawn, NPCSpawn + FVector(100.0f, 0, 0) };
			}
			else if (i == 2) // Nova (delivery)
			{
				NPCSpawn = PickNavSpawnPoint(DistrictRadius * 0.5f) + FVector(0, DistrictRadius * 0.5f, 0);
				NPC->PatrolPoints = { NPCSpawn, NPCSpawn + FVector(100.0f, 0, 0) };
			}

			NPC->ConfigureNPC(false, MissionNames[i], MissionRoles[i], 8 + i);
			if (ANDNPCAIController* AIC = Cast<ANDNPCAIController>(NPC->GetController()))
			{
				AIC->SetPatrolPoints(NPC->PatrolPoints, i < 3);
			}
		}
	}

	// Register mission with the mission system for the HUD marker.
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(GetWorld()))
	{
		if (UNDMissionSystem* Mission = GI->GetSubsystem<UNDMissionSystem>())
		{
			// Find Nova (delivery NPC) as the initial target.
			TArray<AActor*> Found;
			UGameplayStatics::GetAllActorsOfClass(this, ANDNPCCharacter::StaticClass(), Found);
			for (AActor* Actor : Found)
			{
				ANDNPCCharacter* NPC = Cast<ANDNPCCharacter>(Actor);
				if (NPC && NPC->GetMissionRole() == ENPCMissionRole::Delivery)
				{
					// Mission starts at stage 0 (idle) - Mei will accept when player talks to her.
					break;
				}
			}
		}
	}
}

TArray<USplineComponent*> ANDCitySpawner::CollectTrafficRoutes() const
{
	TArray<USplineComponent*> Routes;
	TArray<AActor*> Found;
	UGameplayStatics::GetAllActorsOfClass(this, AActor::StaticClass(), Found);
	for (AActor* Actor : Found)
	{
		if (Actor->ActorHasTag(TEXT("TrafficRoute")))
		{
			if (USplineComponent* Spline = Actor->FindComponentByClass<USplineComponent>())
			{
				Routes.Add(Spline);
			}
		}
	}
	return Routes;
}