// Copyright Neon District Sandbox. Public benchmark repo — original content only.

#include "Benchmark/NDBenchmarkRunner.h"

#include "AI/NDNPCCharacter.h"
#include "AI/NDCitySpawner.h"
#include "Core/NDGameInstance.h"
#include "Core/NDPerfConstants.h"
#include "Player/NDPlayerController.h"
#include "Player/NDCharacter.h"
#include "Systems/NDMissionSystem.h"
#include "Systems/NDWantedSystem.h"
#include "Systems/NDWorldBuilder.h"
#include "Vehicle/NDVehicle.h"

#include "Engine/World.h"
#include "Engine/GameViewportClient.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/DateTime.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "TimerManager.h"
#include "UnrealClient.h"

ANDBenchmarkRunner::ANDBenchmarkRunner()
{
	PrimaryActorTick.bCanEverTick = false;
}

bool ANDBenchmarkRunner::IsBenchmarkMode()
{
	return FParse::Param(FCommandLine::Get(), TEXT("benchmark"));
}

void ANDBenchmarkRunner::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Log, TEXT("[NDBenchmark] Benchmark runner started in '%s'."), *GetWorld()->GetName());

	ReportLines.Add(TEXT("=== Neon District Sandbox — PIE benchmark report ==="));
	const FString MapUrl = GetWorld() ? GetWorld()->URL.Map : TEXT("(no world)");
	ReportLines.Add(FString::Printf(TEXT("Map: %s"), *MapUrl));
	ReportLines.Add(FString::Printf(TEXT("Started: %s"), *FDateTime::Now().ToString(TEXT("%Y-%m-%d %H:%M:%S"))));
	ReportLines.Add(TEXT(""));

	// Give the world builder + spawner time to run before the first check.
	GetWorldTimerManager().SetTimer(PhaseTimerHandle, this, &ANDBenchmarkRunner::RunPhase, 3.0f, false);
}

void ANDBenchmarkRunner::RunPhase()
{
	switch (PhaseIndex)
	{
	case 0: PhaseWorldReady(); break;
	case 1: PhaseActors(); break;
	case 2: PhaseMissionAndWanted(); break;
	case 3: PhaseVehicle(); break;
	case 4: PhaseSaveLoad(); break;
	case 5: PhaseScreenshots(); return; // nested timers drive the flow
	case 6: PhaseFinish(); return; // exits the process
	default: PhaseFinish(); return;
	}

	++PhaseIndex;

	// Next phase after a short delay so the previous action settles.
	const float Delays[] = { 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f };
	const float Delay = (PhaseIndex < 6) ? Delays[PhaseIndex - 1] : 2.0f;
	GetWorldTimerManager().SetTimer(PhaseTimerHandle, this, &ANDBenchmarkRunner::RunPhase, Delay, false);
}

/** Captures 4 staged screenshots back-to-back; each frame is fully rendered. */
void ANDBenchmarkRunner::PhaseScreenshots()
{
	Screenshot(TEXT("01_world_built"));
	GetWorldTimerManager().SetTimer(PhaseTimerHandle, [this]()
	{
		Screenshot(TEXT("02_mission_wanted"));
		GetWorldTimerManager().SetTimer(PhaseTimerHandle, [this]()
		{
			Screenshot(TEXT("03_vehicle"));
			GetWorldTimerManager().SetTimer(PhaseTimerHandle, [this]()
			{
				Screenshot(TEXT("04_final"));
				++PhaseIndex; // skip ahead to finish (PhaseScreenshots already ran)
				GetWorldTimerManager().SetTimer(PhaseTimerHandle, this, &ANDBenchmarkRunner::RunPhase, 2.0f, false);
			}, 1.2f, false);
		}, 1.2f, false);
	}, 1.2f, false);
}

void ANDBenchmarkRunner::Check(bool bPass, const FString& Gate, const FString& Detail)
{
	const FString Line = FString::Printf(TEXT("[%s] %s — %s"), bPass ? TEXT("PASS") : TEXT("FAIL"), *Gate, *Detail);
	ReportLines.Add(Line);
	UE_LOG(LogTemp, Log, TEXT("[NDBenchmark] %s"), *Line);
	if (bPass) { ++PassCount; }
	else { ++FailCount; }
}

void ANDBenchmarkRunner::PhaseWorldReady()
{
	UWorld* World = GetWorld();
	if (!World) { Check(false, TEXT("world"), TEXT("no world")); return; }

	// The world subsystem should have spawned the district builder.
	TArray<AActor*> Builders;
	UGameplayStatics::GetAllActorsOfClass(World, ANDWorldBuilder::StaticClass(), Builders);
	Check(Builders.Num() >= 1, TEXT("world.builder"),
		FString::Printf(TEXT("ANDWorldBuilder actors: %d"), Builders.Num()));

	TArray<AActor*> Spawners;
	UGameplayStatics::GetAllActorsOfClass(World, ANDCitySpawner::StaticClass(), Spawners);
	Check(Spawners.Num() >= 1, TEXT("world.spawner"),
		FString::Printf(TEXT("ANDCitySpawner actors: %d"), Spawners.Num()));
	if (Spawners.Num() > 0)
	{
		Spawner = Spawners[0];
	}

	// Player controller + possessed pawn.
	APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
	Check(PC != nullptr, TEXT("gameplay.player_controller"), TEXT("GetPlayerController(0) exists"));
	Check(PC && PC->GetPawn() != nullptr, TEXT("gameplay.player_pawn"),
		PC && PC->GetPawn() ? *FString::Printf(TEXT("possessed %s"), *PC->GetPawn()->GetClass()->GetName()) : TEXT("no pawn possessed"));

	// GameInstance subsystems. Note: mission/wanted are GameInstanceSubsystems,
	// so they exist on whatever UGameInstance the engine created (the project
	// GameInstanceClass may not resolve in editor `-game` mode; subsystem checks
	// must not depend on that cast).
	UGameInstance* GI = World->GetGameInstance();
	Check(GI != nullptr, TEXT("systems.game_instance"), TEXT("UGameInstance present"));
	if (GI)
	{
		UNDMissionSystem* Mission = GI->GetSubsystem<UNDMissionSystem>();
		Check(Mission != nullptr, TEXT("systems.mission"), TEXT("UNDMissionSystem present"));
		UNDWantedSystem* Wanted = GI->GetSubsystem<UNDWantedSystem>();
		Check(Wanted != nullptr, TEXT("systems.wanted"), TEXT("UNDWantedSystem present"));
	}
}

void ANDBenchmarkRunner::PhaseActors()
{
	UWorld* World = GetWorld();
	if (!World) { Check(false, TEXT("actors"), TEXT("no world")); return; }

	// NPCs: civilians + police (caps from NDPerf).
	TArray<AActor*> NPCs;
	UGameplayStatics::GetAllActorsOfClass(World, ANDNPCCharacter::StaticClass(), NPCs);

	int32 Civilians = 0;
	int32 Police = 0;
	for (AActor* A : NPCs)
	{
		ANDNPCCharacter* NPC = Cast<ANDNPCCharacter>(A);
		if (NPC && NPC->IsPolice()) { ++Police; }
		else { ++Civilians; }
	}

	Check(Civilians >= 10, TEXT("ai.civilians"),
		FString::Printf(TEXT("civilians: %d (>=10)"), Civilians));
	Check(Police >= 1, TEXT("ai.police"),
		FString::Printf(TEXT("police: %d (>=1)"), Police));
	Check(NPCs.Num() >= 12, TEXT("ai.total_npcs"),
		FString::Printf(TEXT("total NPCs: %d (>=12)"), NPCs.Num()));

	// Drivable vehicles.
	TArray<AActor*> Vehicles;
	UGameplayStatics::GetAllActorsOfClass(World, ANDVehicle::StaticClass(), Vehicles);
	Check(Vehicles.Num() >= 1, TEXT("vehicle.count"),
		FString::Printf(TEXT("ANDVehicle actors: %d (>=1)"), Vehicles.Num()));
}

void ANDBenchmarkRunner::PhaseMissionAndWanted()
{
	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	if (!GI) { Check(false, TEXT("mission"), TEXT("no game instance")); Check(false, TEXT("wanted"), TEXT("no game instance")); return; }

	// Mission: accept -> advance -> complete round-trip through the subsystem.
	UNDMissionSystem* Mission = GI->GetSubsystem<UNDMissionSystem>();
	const int32 StageBefore = Mission ? Mission->GetMissionStage() : -1;
	if (Mission)
	{
		Mission->AcceptMission(FText::FromString(TEXT("Entrega el paquete a Nova")), nullptr);
		const int32 StageAfterAccept = Mission->GetMissionStage();
		Mission->CompleteMission();
		const int32 StageAfterComplete = Mission->GetMissionStage();

		Check(StageBefore == 0 && StageAfterAccept >= 1, TEXT("mission.accept"),
			FString::Printf(TEXT("stage %d -> %d"), StageBefore, StageAfterAccept));
		Check(StageAfterComplete >= 3, TEXT("mission.complete"),
			FString::Printf(TEXT("stage after complete: %d"), StageAfterComplete));
	}
	else
	{
		Check(false, TEXT("mission.accept"), TEXT("UNDMissionSystem missing"));
	}

	// Wanted: raise heat via report, then clear.
	UNDWantedSystem* Wanted = GI->GetSubsystem<UNDWantedSystem>();
	if (Wanted)
	{
		const int32 WantedBefore = Wanted->GetWantedLevel();
		Wanted->ReportDetection(1.0f);
		const int32 WantedAfter = Wanted->GetWantedLevel();
		Check(WantedAfter >= WantedBefore, TEXT("wanted.heat"),
			FString::Printf(TEXT("wanted %d -> %d after ReportDetection"), WantedBefore, WantedAfter));

		Wanted->SetWantedLevel(2);
		Check(Wanted->GetWantedLevel() == 2, TEXT("wanted.level2"),
			FString::Printf(TEXT("SetWantedLevel(2) -> %d"), Wanted->GetWantedLevel()));

		Wanted->ClearWanted();
		Check(Wanted->GetWantedLevel() == 0, TEXT("wanted.clear"),
			FString::Printf(TEXT("ClearWanted -> %d"), Wanted->GetWantedLevel()));
	}
	else
	{
		Check(false, TEXT("wanted.heat"), TEXT("UNDWantedSystem missing"));
	}
}

void ANDBenchmarkRunner::PhaseVehicle()
{
	UWorld* World = GetWorld();
	if (!World) { Check(false, TEXT("vehicle"), TEXT("no world")); return; }

	TArray<AActor*> Vehicles;
	UGameplayStatics::GetAllActorsOfClass(World, ANDVehicle::StaticClass(), Vehicles);

	APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
	ANDPlayerController* NDPC = Cast<ANDPlayerController>(PC);

	if (Vehicles.Num() == 0)
	{
		Check(false, TEXT("vehicle.enter"), TEXT("no ANDVehicle to enter"));
		return;
	}

	ANDVehicle* Vehicle = Cast<ANDVehicle>(Vehicles[0]);
	if (!Vehicle || !NDPC)
	{
		Check(false, TEXT("vehicle.enter"), TEXT("vehicle or controller cast failed"));
		return;
	}

	// Enter -> check state -> exit.
	Vehicle->EnterVehicle(NDPC);
	Check(NDPC->IsDriving(), TEXT("vehicle.enter"),
		FString::Printf(TEXT("IsDriving after EnterVehicle: %s"), NDPC->IsDriving() ? TEXT("true") : TEXT("false")));

	if (NDPC->IsDriving())
	{
		// Drive input round-trip (throttle + steering) — must not crash.
		Vehicle->ApplyDriveInput(0.3f, 0.5f);
		Vehicle->SetHandbrake(false);
		Check(true, TEXT("vehicle.drive_input"), TEXT("ApplyDriveInput(0.3, 0.5) + SetHandbrake(false) applied"));
	}

	Vehicle->ExitVehicle(NDPC);
	Check(!NDPC->IsDriving(), TEXT("vehicle.exit"),
		FString::Printf(TEXT("IsDriving after ExitVehicle: %s"), NDPC->IsDriving() ? TEXT("true") : TEXT("false")));
}

void ANDBenchmarkRunner::PhaseSaveLoad()
{
	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UNDGameInstance* NDGI = Cast<UNDGameInstance>(GI);
	if (!GI)
	{
		UE_LOG(LogTemp, Warning, TEXT("[NDBenchmark] GI is nullptr"));
		Check(false, TEXT("save.write"), TEXT("no GameInstance"));
		Check(false, TEXT("save.load"), TEXT("no GameInstance"));
		return;
	}
	if (!NDGI)
	{
		// Log what we actually got for debugging.
		UE_LOG(LogTemp, Warning, TEXT("[NDBenchmark] GI class is: %s, expected UNDGameInstance"), *GI->GetClass()->GetName());
	}
	Check(NDGI != nullptr, TEXT("save.write"),
		NDGI ? TEXT("UNDGameInstance present") :
		TEXT("UNDGameInstance not active; save/load requires the project GameInstance"));
	Check(NDGI != nullptr, TEXT("save.load"), TEXT("same cause as save.write"));
	if (!NDGI)
	{
		return;
	}

	const bool bSaved = NDGI->SaveGame();
	Check(bSaved, TEXT("save.write"), FString::Printf(TEXT("SaveGame() -> %s"), bSaved ? TEXT("true") : TEXT("false")));

	const bool bLoaded = NDGI->LoadGame();
	Check(bLoaded, TEXT("save.load"), FString::Printf(TEXT("LoadGame() -> %s"), bLoaded ? TEXT("true") : TEXT("false")));
}

void ANDBenchmarkRunner::Screenshot(const FString& Name)
{
	UWorld* World = GetWorld();
	if (!World || !World->GetGameViewport())
	{
		UE_LOG(LogTemp, Warning, TEXT("[NDBenchmark] screenshot '%s' skipped (no viewport)"), *Name);
		return;
	}

	const FString FullName = FString::Printf(TEXT("NDBenchmark_%s"), *Name);
	FScreenshotRequest::RequestScreenshot(FullName, false, true);
	UE_LOG(LogTemp, Log, TEXT("[NDBenchmark] Screenshot requested: %s"), *FullName);
}

void ANDBenchmarkRunner::PhaseFinish()
{
	if (bReportWritten)
	{
		return;
	}
	bReportWritten = true;

	ReportLines.Add(TEXT(""));
	ReportLines.Add(FString::Printf(TEXT("=== RESULT: %d passed, %d failed ==="), PassCount, FailCount));

	const FString Dir = FPaths::ProjectSavedDir() / TEXT("Benchmark");
	IFileManager& FM = IFileManager::Get();
	FM.MakeDirectory(*Dir, true);

	const FString ReportPath = Dir / TEXT("NDBenchmarkResult.txt");
	FFileHelper::SaveStringToFile(FString::Join(ReportLines, TEXT("\n")), *ReportPath, FFileHelper::EEncodingOptions::ForceUTF8);

	UE_LOG(LogTemp, Log, TEXT("[NDBenchmark] Report written: %s (%d pass / %d fail)"), *ReportPath, PassCount, FailCount);

	// Exit with a real code so the shell can gate on it.
	FPlatformMisc::RequestExitWithStatus(false, FailCount == 0 ? 0 : 1);
}
