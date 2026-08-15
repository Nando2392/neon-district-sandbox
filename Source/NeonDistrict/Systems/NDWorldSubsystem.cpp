// Copyright Neon District Sandbox. Public benchmark repo — original content only.

#include "Systems/NDWorldSubsystem.h"
#include "Systems/NDWorldBuilder.h"
#include "Audio/NDAudioAnchor.h"
#include "Benchmark/NDBenchmarkRunner.h"

#include "Engine/World.h"

void UNDWorldSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	// Audio anchor lives in every level (menu pad + city ambience).
	InWorld.SpawnActor<ANDAudioAnchor>(FVector::ZeroVector, FRotator::ZeroRotator);

	// Headless benchmark driver: activated with `-benchmark` on the command line.
	// Spawned before the district so the menu screenshot works too; the runner
	// branches on its level name (menu -> screenshot only).
	if (ANDBenchmarkRunner::IsBenchmarkMode())
	{
		InWorld.SpawnActor<ANDBenchmarkRunner>(FVector::ZeroVector, FRotator::ZeroRotator);
		UE_LOG(LogTemp, Log, TEXT("NeonDistrict: benchmark mode active — NDBenchmarkRunner spawned."));
	}

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
}
