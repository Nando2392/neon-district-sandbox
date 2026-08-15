// Copyright Neon District Sandbox. Public benchmark repo — original content only.

#include "Systems/NDWorldSubsystem.h"
#include "Systems/NDWorldBuilder.h"
#include "Benchmark/NDBenchmarkRunner.h"

#include "Engine/World.h"

void UNDWorldSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	if (bDistrictBuilt)
	{
		return;
	}

	const FString LevelName = InWorld.GetName();
	if (LevelName == TEXT("ND_MainMenu"))
	{
		return; // menu level has no district
	}

	// Spawn the procedural district on the main axis.
	InWorld.SpawnActor<ANDWorldBuilder>(FVector::ZeroVector, FRotator::ZeroRotator);
	bDistrictBuilt = true;

	UE_LOG(LogTemp, Log, TEXT("NeonDistrict: WorldSubsystem spawned district builder in '%s'."), *LevelName);

	// Headless benchmark driver: activated with `-benchmark` on the command line.
	// Spawns after the builder so actor counts include the fully populated district.
	if (ANDBenchmarkRunner::IsBenchmarkMode())
	{
		InWorld.SpawnActor<ANDBenchmarkRunner>(FVector::ZeroVector, FRotator::ZeroRotator);
		UE_LOG(LogTemp, Log, TEXT("NeonDistrict: benchmark mode active — NDBenchmarkRunner spawned."));
	}
}
