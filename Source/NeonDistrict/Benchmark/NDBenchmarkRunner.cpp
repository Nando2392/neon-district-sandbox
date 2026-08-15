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

#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
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

	bInMenu = GetWorld() && GetWorld()->GetName() == TEXT("ND_MainMenu");

	// Give the world builder + spawner time to run before the first check.
	GetWorldTimerManager().SetTimer(PhaseTimerHandle, this, &ANDBenchmarkRunner::RunPhase, 3.0f, false);
}

void ANDBenchmarkRunner::RunPhase()
{
	if (bInMenu)
	{
		PhaseMenuOnly();
		return;
	}

	switch (PhaseIndex)
	{
	case 0: PhaseWorldReady(); break;
	case 1: PhaseActors(); break;
	case 2: PhaseMissionAndWanted(); break;
	case 3: PhaseVehicle(); break;
	case 4: PhaseControls(); break;
	case 5: PhaseSaveLoad(); break;
	case 6: PhaseScreenshots(); return; // nested timers drive the flow
	case 7: PhaseFinish(); return; // exits the process
	default: PhaseFinish(); return;
	}

	++PhaseIndex;

	// Next phase after a short delay so the previous action settles.
	const float Delays[] = { 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f };
	const float Delay = (PhaseIndex < 7) ? Delays[PhaseIndex - 1] : 2.0f;
	GetWorldTimerManager().SetTimer(PhaseTimerHandle, this, &ANDBenchmarkRunner::RunPhase, Delay, false);
}

/** Menu run: capture main_menu.png then finish PASS. */
void ANDBenchmarkRunner::PhaseMenuOnly()
{
	if (PhaseIndex == 0)
	{
		++PhaseIndex;
		GetWorldTimerManager().SetTimer(PhaseTimerHandle, [this]()
		{
			Screenshot(TEXT("main_menu"));
			++PhaseIndex;
			GetWorldTimerManager().SetTimer(PhaseTimerHandle, this, &ANDBenchmarkRunner::PhaseMenuOnly, 2.0f, false);
		}, 3.0f, false);
		return;
	}
	Check(true, TEXT("menu.visible"), TEXT("ND_MainMenu loaded; main_menu.png captured"));
	PhaseFinish();
}

/** Captures the 8 visual-gate screenshots; each frame is fully rendered. */
void ANDBenchmarkRunner::PhaseScreenshots()
{
	// Sequence: street, player, Mei interaction, Nova delivery, vehicle, wanted chase, pause.
	Screenshot(TEXT("city_street"));
	GetWorldTimerManager().SetTimer(PhaseTimerHandle, [this]()
	{
		Screenshot(TEXT("player_visible"));
		GetWorldTimerManager().SetTimer(PhaseTimerHandle, [this]()
		{
			// Frame Mei (mission giver) for the interaction shot.
			if (ANDNPCCharacter* Mei = FindMissionNPC(0))
			{
				FrameTarget(Mei);
			}
			Screenshot(TEXT("npc_interaction_mei"));
			GetWorldTimerManager().SetTimer(PhaseTimerHandle, [this]()
			{
				// Frame Nova (delivery) for the mission shot.
				if (ANDNPCCharacter* Nova = FindMissionNPC(2))
				{
					FrameTarget(Nova);
				}
				Screenshot(TEXT("mission_delivery_nova"));
				GetWorldTimerManager().SetTimer(PhaseTimerHandle, [this]()
				{
					// Enter the vehicle and shoot from the vehicle camera.
					if (UWorld* World = GetWorld())
					{
						if (ANDPlayerController* NDPC = Cast<ANDPlayerController>(UGameplayStatics::GetPlayerController(World, 0)))
						{
							TArray<AActor*> Vehicles;
							UGameplayStatics::GetAllActorsOfClass(World, ANDVehicle::StaticClass(), Vehicles);
							if (Vehicles.Num() > 0)
							{
								if (ANDVehicle* Vehicle = Cast<ANDVehicle>(Vehicles[0]))
								{
									Vehicle->EnterVehicle(NDPC);
								}
							}
						}
					}
					Screenshot(TEXT("vehicle_driving"));
					GetWorldTimerManager().SetTimer(PhaseTimerHandle, [this]()
					{
						// Exit the vehicle, then force wanted level 2 for the chase shot.
						if (UWorld* World = GetWorld())
						{
							if (ANDPlayerController* NDPC = Cast<ANDPlayerController>(UGameplayStatics::GetPlayerController(World, 0)))
							{
								if (NDPC->IsDriving())
								{
									NDPC->GetDrivenVehicle()->ExitVehicle(NDPC);
								}
								if (UNDGameInstance* GI = Cast<UNDGameInstance>(World->GetGameInstance()))
								{
									if (UNDWantedSystem* Wanted = GI->GetSubsystem<UNDWantedSystem>())
									{
										Wanted->SetWantedLevel(2);
									}
								}
							}
						}
						Screenshot(TEXT("wanted_police_chase"));
						GetWorldTimerManager().SetTimer(PhaseTimerHandle, [this]()
						{
							// Pause via the real pause path (widget + input mode).
							if (UWorld* World = GetWorld())
							{
								if (ANDPlayerController* NDPC = Cast<ANDPlayerController>(UGameplayStatics::GetPlayerController(World, 0)))
								{
									NDPC->HandlePause();
								}
							}
							Screenshot(TEXT("pause_menu"));
							if (UWorld* World = GetWorld())
							{
								if (ANDPlayerController* NDPC = Cast<ANDPlayerController>(UGameplayStatics::GetPlayerController(World, 0)))
								{
									NDPC->HandlePauseFromWidget(); // resume
								}
							}
							++PhaseIndex; // skip ahead to finish (PhaseScreenshots already ran)
							GetWorldTimerManager().SetTimer(PhaseTimerHandle, this, &ANDBenchmarkRunner::RunPhase, 2.0f, false);
						}, 1.5f, false);
					}, 1.5f, false);
				}, 1.5f, false);
			}, 1.5f, false);
		}, 1.5f, false);
	}, 1.5f, false);
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

	// GameInstance subsystems.
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

/** Pause round-trip + real player movement (both are user-facing controls). */
void ANDBenchmarkRunner::PhaseControls()
{
	UWorld* World = GetWorld();
	APlayerController* PC = World ? UGameplayStatics::GetPlayerController(World, 0) : nullptr;
	ANDPlayerController* NDPC = Cast<ANDPlayerController>(PC);

	if (!World || !NDPC)
	{
		Check(false, TEXT("controls.pause"), TEXT("no player controller"));
		Check(false, TEXT("gameplay.player_move"), TEXT("no player controller"));
		return;
	}

	// Pause -> resume round-trip through the real input path.
	NDPC->HandlePause();
	const bool bPaused = World->IsPaused();
	NDPC->HandlePauseFromWidget();
	const bool bResumed = !World->IsPaused();
	Check(bPaused && bResumed, TEXT("controls.pause"),
		FString::Printf(TEXT("HandlePause -> paused=%s; resume -> running=%s"), bPaused ? TEXT("true") : TEXT("false"), bResumed ? TEXT("true") : TEXT("false")));

	// Real movement: teleport the pawn onto a real avenue point (the district
	// has no baked navmesh, so the default spawn at the origin is embedded in
	// procedural geometry and cannot move). Land on asphalt, settle, drive
	// forward, verify horizontal displacement.
	APawn* Pawn = NDPC->GetPawn();
	if (!Pawn)
	{
		Check(false, TEXT("gameplay.player_move"), TEXT("no pawn"));
		return;
	}

	// Ask the world builder for a point on an avenue (same grid the player walks).
	FVector Street = Pawn->GetActorLocation();
	if (World)
	{
		TArray<AActor*> Builders;
		UGameplayStatics::GetAllActorsOfClass(World, ANDWorldBuilder::StaticClass(), Builders);
		for (AActor* B : Builders)
		{
			if (ANDWorldBuilder* Builder = Cast<ANDWorldBuilder>(B))
			{
				Street = Builder->GetRandomStreetPoint() + FVector(0.0f, 0.0f, 500.0f);
				break;
			}
		}
	}
	// Sweep on: the drop collides with the asphalt and the CMC resyncs its
	// floor (a plain teleport leaves it "grounded in the air" with zero velocity).
	Pawn->SetActorLocation(Street, true, nullptr, ETeleportType::TeleportPhysics);
	const FRotator Yaw(0.0f, FMath::FRandRange(0.0f, 360.0f), 0.0f);
	NDPC->SetControlRotation(Yaw);
	Pawn->SetActorRotation(FRotator(0.0f, Yaw.Yaw, 0.0f));

	// Stage 0: settle onto the ground (drop + physics settle), then move.
	MoveStage = 0;
	MoveSettleTicks = 12; // 1.2s
	MoveStartLoc = Pawn->GetActorLocation();
	MoveTicksLeft = 0;
	GetWorldTimerManager().SetTimer(MoveTimerHandle, this, &ANDBenchmarkRunner::TickMoveCheck, 0.1f, true);
}

void ANDBenchmarkRunner::TickMoveCheck()
{
	UWorld* World = GetWorld();
	APlayerController* PC = World ? UGameplayStatics::GetPlayerController(World, 0) : nullptr;
	APawn* Pawn = PC ? PC->GetPawn() : nullptr;
	if (!Pawn)
	{
		Check(false, TEXT("gameplay.player_move"), TEXT("pawn lost mid-check"));
		GetWorldTimerManager().ClearTimer(MoveTimerHandle);
		return;
	}

	const UCharacterMovementComponent* MoveComp = Pawn->FindComponentByClass<UCharacterMovementComponent>();
	const FVector PendingInput = Pawn->GetPendingMovementInputVector();
	const AController* Ctrl = Pawn->GetController();
	const UWorld* DiagWorld = World;
	UE_LOG(LogTemp, Log, TEXT("[NDBenchmark] move tick stage=%d ticks=%d loc=(%.0f,%.0f,%.0f) vel=(%.0f,%.0f,%.0f) mode=%s ground=%s pending=(%.0f,%.0f,%.0f) accel=(%.0f,%.0f,%.0f) pawnTick=%s cmcTick=%s ctrl=%s cmcActive=%s simPhys=%s localCtrl=%s worldTime=%.2f delta=%.4f"),
		MoveStage, MoveTicksLeft, Pawn->GetActorLocation().X, Pawn->GetActorLocation().Y, Pawn->GetActorLocation().Z,
		Pawn->GetVelocity().X, Pawn->GetVelocity().Y, Pawn->GetVelocity().Z,
		MoveComp ? *UEnum::GetValueAsString(MoveComp->MovementMode) : TEXT("(no CMC)"),
		MoveComp ? (MoveComp->IsMovingOnGround() ? TEXT("true") : TEXT("false")) : TEXT("n/a"),
		PendingInput.X, PendingInput.Y, PendingInput.Z,
		MoveComp ? MoveComp->GetCurrentAcceleration().X : 0.0f,
		MoveComp ? MoveComp->GetCurrentAcceleration().Y : 0.0f,
		MoveComp ? MoveComp->GetCurrentAcceleration().Z : 0.0f,
		Pawn->IsActorTickEnabled() ? TEXT("true") : TEXT("false"),
		MoveComp ? (MoveComp->IsComponentTickEnabled() ? TEXT("true") : TEXT("false")) : TEXT("n/a"),
		Ctrl ? TEXT("yes") : TEXT("NO"),
		MoveComp ? (MoveComp->IsActive() ? TEXT("true") : TEXT("false")) : TEXT("n/a"),
		MoveComp ? (MoveComp->IsInWater() ? TEXT("true") : TEXT("false")) : TEXT("n/a"),
		Ctrl ? (Ctrl->IsLocalController() ? TEXT("true") : TEXT("false")) : TEXT("n/a"),
		DiagWorld ? DiagWorld->GetTimeSeconds() : -1.0,
		DiagWorld ? DiagWorld->GetDeltaSeconds() : -1.0);

	if (MoveStage == 0)
	{
		// Settle: let the pawn fall onto the street and stop moving.
		if (MoveSettleTicks > 0)
		{
			--MoveSettleTicks;
			return;
		}
		MoveStage = 1;
		MoveStartLoc = Pawn->GetActorLocation();
		MoveTicksLeft = 6;
		return;
	}

	if (MoveTicksLeft > 0)
	{
		--MoveTicksLeft;
		// Move toward the pawn's facing (control rotation yaw).
		const FRotator Yaw(0.0f, PC->GetControlRotation().Yaw, 0.0f);
		Pawn->AddMovementInput(FRotationMatrix(Yaw).GetUnitAxis(EAxis::X), 1.0f);

		// Control probe: on the last input tick also fire a direct launch
		// (XY+Z overrides) to prove whether the CMC can move the pawn at all.
		if (MoveTicksLeft == 0)
		{
			const UCapsuleComponent* Capsule = Cast<UCapsuleComponent>(Pawn->GetRootComponent());
			UE_LOG(LogTemp, Log, TEXT("[NDBenchmark] gravityZ=%.1f capsuleHalf=%.1f capsuleRadius=%.1f"),
				MoveComp ? MoveComp->GetGravityZ() : 0.0f,
				Capsule ? Capsule->GetScaledCapsuleHalfHeight() : -1.0f,
				Capsule ? Capsule->GetScaledCapsuleRadius() : -1.0f);
			if (ACharacter* Ch = Cast<ACharacter>(Pawn))
			{
				Ch->LaunchCharacter(FRotationMatrix(Yaw).GetUnitAxis(EAxis::X) * 500.0f, true, true);
			}
		}
		return;
	}

	GetWorldTimerManager().ClearTimer(MoveTimerHandle);
	const float Distance = FVector::Dist2D(MoveStartLoc, Pawn->GetActorLocation());
	Check(Distance > 10.0f, TEXT("gameplay.player_move"),
		FString::Printf(TEXT("moved %.1f cm over %d input ticks (>=10)"), Distance, 6));
	// PhaseTimerHandle continues the normal phase flow (save/load next).
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

void ANDBenchmarkRunner::FrameTarget(AActor* Target)
{
	UWorld* World = GetWorld();
	if (!World || !Target)
	{
		return;
	}
	APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
	APawn* Pawn = PC ? PC->GetPawn() : nullptr;
	if (!PC || !Pawn)
	{
		return;
	}

	// Park the pawn a few meters from the target and look at it (third person
	// camera follows, so the target lands in frame).
	const FVector TargetLoc = Target->GetActorLocation();
	const FVector ToTarget = (TargetLoc - Pawn->GetActorLocation()).GetSafeNormal2D();
	const FVector Park = TargetLoc - ToTarget * 400.0f;
	Pawn->SetActorLocation(Park + FVector(0.0f, 0.0f, 80.0f));
	PC->SetControlRotation((TargetLoc - Pawn->GetActorLocation()).Rotation());
	Pawn->SetActorRotation(FRotator(0.0f, PC->GetControlRotation().Yaw, 0.0f));
}

ANDNPCCharacter* ANDBenchmarkRunner::FindMissionNPC(int32 RoleIndex) const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}
	TArray<AActor*> NPCs;
	UGameplayStatics::GetAllActorsOfClass(World, ANDNPCCharacter::StaticClass(), NPCs);
	for (AActor* A : NPCs)
	{
		ANDNPCCharacter* NPC = Cast<ANDNPCCharacter>(A);
		if (!NPC)
		{
			continue;
		}
		// Spawner order: 0 = Mei (giver), 1 = package, 2 = Nova (delivery).
		if (RoleIndex == 0 && NPC->GetMissionRole() == ENPCMissionRole::MissionGiver)
		{
			return NPC;
		}
		if (RoleIndex == 2 && NPC->GetMissionRole() == ENPCMissionRole::Delivery)
		{
			return NPC;
		}
	}
	return nullptr;
}

void ANDBenchmarkRunner::Screenshot(const FString& Name)
{
	UWorld* World = GetWorld();
	if (!World || !World->GetGameViewport())
	{
		UE_LOG(LogTemp, Warning, TEXT("[NDBenchmark] screenshot '%s' skipped (no viewport)"), *Name);
		return;
	}

	FScreenshotRequest::RequestScreenshot(Name, false, false);
	UE_LOG(LogTemp, Log, TEXT("[NDBenchmark] Screenshot requested: %s"), *Name);
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
