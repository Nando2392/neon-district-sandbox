// Copyright Neon District Sandbox. Public benchmark repo — original content only

#include "Player/NDPlayerController.h"
#include "Player/NDCharacter.h"
#include "Vehicle/NDVehicle.h"
#include "UI/NDHUDWidget.h"
#include "UI/NDPauseWidget.h"
#include "UI/NDMainMenuWidget.h"
#include "Core/NDGameInstance.h"
#include "Core/NDSaveGame.h"
#include "Blueprint/UserWidget.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "Engine/World.h"
#include "Components/InputComponent.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Character.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"
#include "Player/NDInteractable.h"
#include "Combat/NDWeaponProjectile.h"
#include "Systems/NDWantedSystem.h"
#include "Audio/NDAudioManager.h"

ANDPlayerController::ANDPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
}

UNDHUDWidget* ANDPlayerController::GetHUDWidget() const
{
	return Cast<UNDHUDWidget>(HUDWidget);
}

void ANDPlayerController::BeginPlay()
{
	Super::BeginPlay();

	FString LevelName = GetWorld() ? GetWorld()->GetName() : FString();

	// Main menu check - not active in benchmark
	if (LevelName == TEXT("ND_MainMenu"))
	{
		// Show the procedural main menu (Play / Continue / Quit).
		if (UNDMainMenuWidget* Menu = CreateWidget<UNDMainMenuWidget>(this, UNDMainMenuWidget::StaticClass()))
		{
			Menu->AddToViewport(10);
			SetInputMode(FInputModeUIOnly());
			bShowMouseCursor = true;
		}
		return;
	}

	// Gameplay level: setup input
	SetupGameplayInput();

	// Create the HUD (objective, wanted, prompt, notifications).
	if (!HUDWidget)
	{
		if (UNDHUDWidget* HUD = CreateWidget<UNDHUDWidget>(this, UNDHUDWidget::StaticClass()))
		{
			HUD->AddToViewport(5);
			HUDWidget = HUD;
		}
	}
}

void ANDPlayerController::SetupGameplayInput()
{
	// First try Enhanced Input setup
	bool bEnhancedInputReady = TrySetupEnhancedInput();

	if (!bEnhancedInputReady)
	{
		SetupFallbackInput();
	}

	// Keep a direct character ref
	PlayerCharacter = Cast<ANDCharacter>(GetPawn());
}

bool ANDPlayerController::TrySetupEnhancedInput()
{
	// Create InputMappingContext
	InputContext = NewObject<UInputMappingContext>(this, TEXT("ND_DefaultContext"));

	// Create InputActions
	IA_Move = NewObject<UInputAction>(this, FName("IA_Move"));
	if (IA_Move) IA_Move->ValueType = EInputActionValueType::Axis2D;

	IA_Look = NewObject<UInputAction>(this, FName("IA_Look"));
	if (IA_Look) IA_Look->ValueType = EInputActionValueType::Axis2D;

	IA_Jump = NewObject<UInputAction>(this, FName("IA_Jump"));
	if (IA_Jump) IA_Jump->ValueType = EInputActionValueType::Boolean;

	IA_Sprint = NewObject<UInputAction>(this, FName("IA_Sprint"));
	if (IA_Sprint) IA_Sprint->ValueType = EInputActionValueType::Boolean;

	IA_Interact = NewObject<UInputAction>(this, FName("IA_Interact"));
	if (IA_Interact) IA_Interact->ValueType = EInputActionValueType::Boolean;

	IA_Vehicle = NewObject<UInputAction>(this, FName("IA_Vehicle"));
	if (IA_Vehicle) IA_Vehicle->ValueType = EInputActionValueType::Boolean;

	IA_Pause = NewObject<UInputAction>(this, FName("IA_Pause"));
	if (IA_Pause) IA_Pause->ValueType = EInputActionValueType::Boolean;

	IA_QuickSave = NewObject<UInputAction>(this, FName("IA_QuickSave"));
	if (IA_QuickSave) IA_QuickSave->ValueType = EInputActionValueType::Boolean;

	IA_QuickLoad = NewObject<UInputAction>(this, FName("IA_QuickLoad"));
	if (IA_QuickLoad) IA_QuickLoad->ValueType = EInputActionValueType::Boolean;

	IA_Fire = NewObject<UInputAction>(this, FName("IA_Fire"));
	if (IA_Fire) IA_Fire->ValueType = EInputActionValueType::Boolean;

	// Add to local player subsystem
	ULocalPlayer* LP = GetLocalPlayer();
	if (LP)
	{
		UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
		if (Subsystem)
		{
			Subsystem->AddMappingContext(InputContext, 0);
		}
	}

	// Bind Enhanced Input
	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent);
	if (EIC)
	{
		BindEnhancedInput(EIC);
		return true;
	}

	return false;
}

void ANDPlayerController::BindEnhancedInput(UEnhancedInputComponent* EIC)
{
	EIC->BindAction(IA_Move, ETriggerEvent::Triggered, this, &ANDPlayerController::HandleMove);
	EIC->BindAction(IA_Look, ETriggerEvent::Triggered, this, &ANDPlayerController::HandleLook);
	EIC->BindAction(IA_Jump, ETriggerEvent::Started, this, &ANDPlayerController::HandleJumpStart);
	EIC->BindAction(IA_Jump, ETriggerEvent::Completed, this, &ANDPlayerController::HandleJumpStop);
	EIC->BindAction(IA_Sprint, ETriggerEvent::Started, this, &ANDPlayerController::HandleSprintStart);
	EIC->BindAction(IA_Sprint, ETriggerEvent::Completed, this, &ANDPlayerController::HandleSprintStop);
	EIC->BindAction(IA_Interact, ETriggerEvent::Started, this, &ANDPlayerController::HandleInteract);
	EIC->BindAction(IA_Vehicle, ETriggerEvent::Started, this, &ANDPlayerController::HandleEnterExitVehicle);
	EIC->BindAction(IA_Pause, ETriggerEvent::Started, this, &ANDPlayerController::HandlePause);
	EIC->BindAction(IA_QuickSave, ETriggerEvent::Started, this, &ANDPlayerController::HandleQuickSave);
	EIC->BindAction(IA_QuickLoad, ETriggerEvent::Started, this, &ANDPlayerController::HandleQuickLoad);
	EIC->BindAction(IA_Fire, ETriggerEvent::Started, this, &ANDPlayerController::HandleFire);

	SetupInputMappings();
}

void ANDPlayerController::SetupInputMappings()
{
	if (!InputContext) return;

	// Movement
	InputContext->MapKey(IA_Move, EKeys::W);
	InputContext->MapKey(IA_Move, EKeys::S);
	InputContext->MapKey(IA_Move, EKeys::A);
	InputContext->MapKey(IA_Move, EKeys::D);
	InputContext->MapKey(IA_Move, EKeys::Up);
	InputContext->MapKey(IA_Move, EKeys::Down);
	InputContext->MapKey(IA_Move, EKeys::Left);
	InputContext->MapKey(IA_Move, EKeys::Right);

	// Look
	InputContext->MapKey(IA_Look, EKeys::MouseX);
	InputContext->MapKey(IA_Look, EKeys::MouseY);

	// Actions
	InputContext->MapKey(IA_Jump, EKeys::SpaceBar);
	InputContext->MapKey(IA_Sprint, EKeys::LeftShift);
	InputContext->MapKey(IA_Interact, EKeys::E);
	InputContext->MapKey(IA_Vehicle, EKeys::F);
	InputContext->MapKey(IA_Pause, EKeys::Escape);
	InputContext->MapKey(IA_QuickSave, EKeys::F5);
	InputContext->MapKey(IA_QuickLoad, EKeys::F9);
	InputContext->MapKey(IA_Fire, EKeys::LeftMouseButton);
}

void ANDPlayerController::SetupFallbackInput()
{
	bUsingFallback = true;

	InputComponent->BindAxis("MoveForward", this, &ANDPlayerController::HandleMoveForwardFallback);
	InputComponent->BindAxis("MoveRight", this, &ANDPlayerController::HandleMoveRightFallback);
	InputComponent->BindAxis("Turn", this, &ANDPlayerController::HandleLookHorizontalFallback);
	InputComponent->BindAxis("LookUp", this, &ANDPlayerController::HandleLookVerticalFallback);

	InputComponent->BindAction("Jump", IE_Pressed, this, &ANDPlayerController::HandleJumpStart);
	InputComponent->BindAction("Jump", IE_Released, this, &ANDPlayerController::HandleJumpStop);
	InputComponent->BindAction("Sprint", IE_Pressed, this, &ANDPlayerController::HandleSprintStart);
	InputComponent->BindAction("Sprint", IE_Released, this, &ANDPlayerController::HandleSprintStop);
	InputComponent->BindAction("Interact", IE_Pressed, this, &ANDPlayerController::HandleInteract);
	InputComponent->BindAction("EnterVehicle", IE_Pressed, this, &ANDPlayerController::HandleEnterExitVehicle);
	InputComponent->BindAction("Pause", IE_Pressed, this, &ANDPlayerController::HandlePause);
	InputComponent->BindAction("QuickSave", IE_Pressed, this, &ANDPlayerController::HandleQuickSave);
	InputComponent->BindAction("QuickLoad", IE_Pressed, this, &ANDPlayerController::HandleQuickLoad);
	InputComponent->BindAction("Fire", IE_Pressed, this, &ANDPlayerController::HandleFire);
}

void ANDPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	UpdateInteractionTarget();
}

void ANDPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
}

// --- Fallback handlers ---

void ANDPlayerController::HandleMoveForwardFallback(float AxisValue)
{
	if (bUsingFallback)
	{
		HandleMoveAxis(FVector(1, 0, 0), AxisValue);
	}
}

void ANDPlayerController::HandleMoveRightFallback(float AxisValue)
{
	if (bUsingFallback)
	{
		HandleMoveAxis(FVector(0, 1, 0), AxisValue);
	}
}

void ANDPlayerController::HandleLookHorizontalFallback(float AxisValue)
{
	if (bUsingFallback)
	{
		AddYawInput(AxisValue);
	}
}

void ANDPlayerController::HandleLookVerticalFallback(float AxisValue)
{
	if (bUsingFallback)
	{
		AddPitchInput(-AxisValue);
	}
}

void ANDPlayerController::HandleMoveAxis(const FVector& Direction, float Value)
{
	if (!Value) return;
	
	if (bIsDriving && DrivenVehicle)
	{
		float Steering = Value;
		DrivenVehicle->ApplyDriveInput(Value, Steering);
		return;
	}

	ACharacter* Char = Cast<ACharacter>(GetPawn());
	if (Char)
	{
		Char->AddMovementInput(Direction, Value);
	}
}

// --- Enhanced Input handlers ---

void ANDPlayerController::HandleMove(const FInputActionValue& Value)
{
	FVector2D Axis = Value.Get<FVector2D>();

	if (bIsDriving && DrivenVehicle)
	{
		DrivenVehicle->ApplyDriveInput(Axis.Y, Axis.X);
		return;
	}

	ACharacter* Char = Cast<ACharacter>(GetPawn());
	if (Char)
	{
		FRotator Yaw(0.0f, GetControlRotation().Yaw, 0.0f);
		FVector Forward = FRotationMatrix(Yaw).GetUnitAxis(EAxis::X);
		FVector Right = FRotationMatrix(Yaw).GetUnitAxis(EAxis::Y);
		Char->AddMovementInput(Forward, Axis.X);
		Char->AddMovementInput(Right, Axis.Y);
	}
}

void ANDPlayerController::HandleLook(const FInputActionValue& Value)
{
	FVector2D Axis = Value.Get<FVector2D>();
	AddYawInput(Axis.X);
	AddPitchInput(-Axis.Y);
}

void ANDPlayerController::HandleJumpStart()
{
	if (bIsDriving && DrivenVehicle)
	{
		DrivenVehicle->SetHandbrake(true);
		return;
	}
	if (PlayerCharacter)
	{
		PlayerCharacter->Jump();
	}
}

void ANDPlayerController::HandleJumpStop()
{
	if (bIsDriving && DrivenVehicle)
	{
		DrivenVehicle->SetHandbrake(false);
		return;
	}
	if (PlayerCharacter)
	{
		PlayerCharacter->StopJumping();
	}
}

void ANDPlayerController::HandleSprintStart()
{
	if (!bIsDriving && PlayerCharacter)
	{
		PlayerCharacter->SetSprinting(true);
	}
}

void ANDPlayerController::HandleSprintStop()
{
	if (PlayerCharacter)
	{
		PlayerCharacter->SetSprinting(false);
	}
}

void ANDPlayerController::HandleInteract()
{
	// Interact with the current target (NPC mission giver, package, delivery,
	// vehicle seat, pickup). The target exposes INDIInteractable.
	if (CurrentInteractable)
	{
		if (INDIInteractable* Interactable = Cast<INDIInteractable>(CurrentInteractable))
		{
			Interactable->Execute_Interact(CurrentInteractable, this);
		}
	}
}

void ANDPlayerController::HandleEnterExitVehicle()
{
	// Enter the nearest drivable vehicle when on foot; exit when driving.
	if (bIsDriving && DrivenVehicle)
	{
		DrivenVehicle->ExitVehicle(this);
		return;
	}

	if (UWorld* World = GetWorld())
	{
		TArray<AActor*> Vehicles;
		UGameplayStatics::GetAllActorsOfClass(World, ANDVehicle::StaticClass(), Vehicles);

		APawn* MyPawn = GetPawn();
		const FVector PlayerLoc = MyPawn ? MyPawn->GetActorLocation() : FVector::ZeroVector;

		ANDVehicle* Nearest = nullptr;
		float NearestDist = 420.0f; // interact radius
		for (AActor* V : Vehicles)
		{
			if (!V)
			{
				continue;
			}
			const float Dist = FVector::Dist2D(PlayerLoc, V->GetActorLocation());
			if (Dist < NearestDist)
			{
				NearestDist = Dist;
				Nearest = Cast<ANDVehicle>(V);
			}
		}

		if (Nearest)
		{
			Nearest->EnterVehicle(this);
		}
	}
}

void ANDPlayerController::HandleQuickSave()
{
	// Quick save (F5): snapshot the player into the persistent slot via the
	// project GameInstance (UNDGameInstance). The save is written with
	// SaveGameToSlot so it survives restarts.
	if (UNDGameInstance* GI = GetGameInstance<UNDGameInstance>())
	{
		const bool bSaved = GI->SaveGame();
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, bSaved ? FColor::Green : FColor::Red,
				bSaved ? TEXT("Partida guardada (F5)") : TEXT("Error al guardar"));
		}
	}
	else if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("No GameInstance para guardar"));
	}
}

void ANDPlayerController::HandleQuickLoad()
{
	// Quick load (F9): restore the snapshot (position + mission/wanted state
	// lives in GameInstance subsystems, so a fresh snapshot is enough).
	if (UNDGameInstance* GI = GetGameInstance<UNDGameInstance>())
	{
		const bool bLoaded = GI->LoadGame();
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, bLoaded ? FColor::Green : FColor::Yellow,
				bLoaded ? TEXT("Partida cargada (F9)") : TEXT("No hay partida guardada"));
		}

		// Teleport the possessed pawn to the saved location.
		if (bLoaded)
		{
			if (UNDSaveGame* Save = GI->GetMutableSave())
			{
				if (APawn* MyPawn = GetPawn())
				{
					MyPawn->SetActorLocation(Save->PlayerLocation + FVector(0.0f, 0.0f, 80.0f));
					MyPawn->SetActorRotation(FRotator(0.0f, Save->PlayerYaw, 0.0f));
					SetControlRotation(FRotator(0.0f, Save->PlayerYaw, 0.0f));
				}
			}
		}
	}
	else if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("No GameInstance para cargar"));
	}
}

void ANDPlayerController::EquipWeapon(int32 InitialAmmo)
{
	bWeaponEquipped = true;
	WeaponAmmo = FMath::Max(InitialAmmo, WeaponAmmo);
	if (!PlayerCharacter)
	{
		PlayerCharacter = Cast<ANDCharacter>(GetPawn());
	}
	if (PlayerCharacter)
	{
		PlayerCharacter->SetWeaponVisible(true);
	}
	if (HUDWidget)
	{
		HUDWidget->SetWeaponState(true, WeaponAmmo);
		HUDWidget->ShowNotification(FText::FromString(TEXT("Blaster urbano equipado — Mouse izquierdo para disparar")));
	}
}

void ANDPlayerController::HandleFire()
{
	FireWeapon();
}

bool ANDPlayerController::FireWeapon()
{
	if (!bWeaponEquipped || WeaponAmmo <= 0 || bIsDriving)
	{
		return false;
	}
	UWorld* World = GetWorld();
	APawn* ControlledPawn = GetPawn();
	if (!World || !ControlledPawn || !PlayerCameraManager)
	{
		return false;
	}

	return FireWeaponFrom(PlayerCameraManager->GetCameraLocation(), PlayerCameraManager->GetCameraRotation().Vector());
}

bool ANDPlayerController::FireWeaponFrom(const FVector& Origin, const FVector& DirectionIn)
{
	if (!bWeaponEquipped || WeaponAmmo <= 0 || bIsDriving)
	{
		return false;
	}
	UWorld* World = GetWorld();
	APawn* ControlledPawn = GetPawn();
	if (!World || !ControlledPawn)
	{
		return false;
	}

	const FVector Direction = DirectionIn.GetSafeNormal();
	const FVector TraceEnd = Origin + Direction * 6500.0f;
	FHitResult Hit;
	FCollisionQueryParams Params(FName(TEXT("ND_WeaponFire")), /*bTraceComplex=*/false);
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(ControlledPawn);
	FCollisionObjectQueryParams PawnObjects;
	PawnObjects.AddObjectTypesToQuery(ECC_Pawn);
	bool bHit = World->LineTraceSingleByObjectType(Hit, Origin, TraceEnd, PawnObjects, Params);
	if (!bHit)
	{
		bHit = World->LineTraceSingleByChannel(Hit, Origin, TraceEnd, ECC_Visibility, Params);
	}
	if (bHit && Hit.GetActor())
	{
		UGameplayStatics::ApplyPointDamage(Hit.GetActor(), 34.0f, Direction, Hit,
			GetInstigatorController(), ControlledPawn, nullptr);
		if (UPrimitiveComponent* HitComp = Hit.GetComponent())
		{
			if (HitComp->IsSimulatingPhysics())
			{
				HitComp->AddImpulseAtLocation(Direction * 65000.0f, Hit.ImpactPoint);
			}
		}
		if (ACharacter* HitCharacter = Cast<ACharacter>(Hit.GetActor()))
		{
			HitCharacter->LaunchCharacter(Direction.GetSafeNormal2D() * 260.0f + FVector(0.0f, 0.0f, 70.0f), true, true);
		}
		UE_LOG(LogTemp, Log, TEXT("NeonDistrict: weapon hitscan hit actor=%s at=(%.0f,%.0f,%.0f)"),
			*Hit.GetActor()->GetName(), Hit.ImpactPoint.X, Hit.ImpactPoint.Y, Hit.ImpactPoint.Z);
	}
	const FVector ProjectileSpawnLocation = Origin + Direction * 120.0f + FVector(0.0f, 0.0f, -18.0f);
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = ControlledPawn;
	SpawnParams.Instigator = ControlledPawn;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ANDWeaponProjectile* Projectile = World->SpawnActor<ANDWeaponProjectile>(ANDWeaponProjectile::StaticClass(),
		ProjectileSpawnLocation, Direction.Rotation(), SpawnParams);
	if (!Projectile)
	{
		return false;
	}
	Projectile->Launch(Direction, ControlledPawn);
	--WeaponAmmo;
	if (HUDWidget)
	{
		HUDWidget->SetWeaponState(true, WeaponAmmo);
	}
	if (UNDGameInstance* GI = GetGameInstance<UNDGameInstance>())
	{
		if (UNDAudioManager* Audio = GI->GetAudioManager())
		{
			Audio->PlayAlert();
		}
		if (UNDWantedSystem* Wanted = GI->GetSubsystem<UNDWantedSystem>())
		{
			Wanted->ReportDetection(0.45f);
		}
	}
	UE_LOG(LogTemp, Log, TEXT("NeonDistrict: weapon fired ammo=%d projectile=%s"), WeaponAmmo, *Projectile->GetName());
	return true;
}

void ANDPlayerController::UpdateInteractionTarget()
{
	// Find the interactable under the camera crosshair: line trace from the
	// camera forward, biased a bit toward the player so the player can also
	// interact by looking slightly down at a target near their feet.
	if (bIsDriving)
	{
		CurrentInteractable = nullptr;
		if (HUDWidget)
		{
			HUDWidget->SetInteractionPrompt(FText::GetEmpty());
		}
		return;
	}

	UWorld* World = GetWorld();
	APlayerCameraManager* CamMgr = PlayerCameraManager;
	if (!World || !CamMgr || !PlayerCharacter)
	{
		CurrentInteractable = nullptr;
		return;
	}

	AActor* Best = nullptr;
	float BestScore = TNumericLimits<float>::Max();

	// Direct camera trace.
	{
		const FVector Start = CamMgr->GetCameraLocation();
		const FVector End = Start + CamMgr->GetCameraRotation().Vector() * 600.0f;
		FHitResult Hit;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);
		Params.AddIgnoredActor(GetPawn());
		if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
		{
			if (AActor* HitActor = Hit.GetActor())
			{
				if (HitActor->Implements<UNDIInteractable>())
				{
					Best = HitActor;
					BestScore = Hit.Distance;
				}
			}
		}
	}

	// Proximity fallback for NPCs/vehicles in a short radius around the player.
	{
		const FVector PlayerLoc = GetPawn() ? GetPawn()->GetActorLocation() : FVector::ZeroVector;
		const FVector PlayerFwd = GetControlRotation().Vector();
		TArray<AActor*> Candidates;
		UGameplayStatics::GetAllActorsWithInterface(World, UNDIInteractable::StaticClass(), Candidates);
		for (AActor* A : Candidates)
		{
			if (!A || A == Best)
			{
				continue;
			}
			const FVector ToTarget = (A->GetActorLocation() - PlayerLoc);
			const float Dist = ToTarget.Size2D();
			if (Dist > 320.0f)
			{
				continue;
			}
			// Favor targets in front of the player.
			const float Dot = FVector::DotProduct(ToTarget.GetSafeNormal2D(), PlayerFwd.GetSafeNormal2D());
			const float Score = Dist - Dot * 200.0f;
			if (Score < BestScore)
			{
				BestScore = Score;
				Best = A;
			}
		}
	}

	CurrentInteractable = Best;

	// Update the HUD prompt.
	if (HUDWidget)
	{
		FText Prompt = FText::GetEmpty();
		if (Best)
		{
			if (INDIInteractable* Interactable = Cast<INDIInteractable>(Best))
			{
				Prompt = INDIInteractable::Execute_GetInteractionPrompt(Best);
			}
		}
		HUDWidget->SetInteractionPrompt(Prompt);
	}
}

void ANDPlayerController::SetDrivingState(ANDVehicle* Vehicle, bool bEntering)
{
	DrivenVehicle = bEntering ? Vehicle : nullptr;
	bIsDriving = bEntering;
}

void ANDPlayerController::RestoreCharacterFromVehicle()
{
	DrivenVehicle = nullptr;
	bIsDriving = false;
}

// --- Public test functions ---

void ANDPlayerController::TestMoveForward(float Value)
{
	HandleMoveAxis(FVector(1, 0, 0), Value);
}

void ANDPlayerController::TestMoveRight(float Value)
{
	HandleMoveAxis(FVector(0, 1, 0), Value);
}

void ANDPlayerController::TestJump()
{
	HandleJumpStart();
}

void ANDPlayerController::TestInteract()
{
	HandleInteract();
}

// --- Pause functions (needed by NDBenchmarkRunner and NDPauseWidget) ---

void ANDPlayerController::HandlePause()
{
	UWorld* World = GetWorld();
	if (!World) return;

	if (World->IsPaused())
	{
		UGameplayStatics::SetGamePaused(World, false);
		if (PauseWidget)
		{
			PauseWidget->RemoveFromParent();
			PauseWidget = nullptr;
		}
		SetInputMode(FInputModeGameOnly());
		bShowMouseCursor = false;
	}
	else
	{
		UGameplayStatics::SetGamePaused(World, true);
		if (!PauseWidget)
		{
			if (UNDPauseWidget* Pause = CreateWidget<UNDPauseWidget>(this, UNDPauseWidget::StaticClass()))
			{
				Pause->AddToViewport(20);
				PauseWidget = Pause;
			}
		}
		SetInputMode(FInputModeGameAndUI());
		bShowMouseCursor = true;
	}
}

void ANDPlayerController::HandlePauseFromWidget()
{
	HandlePause(); // resume (world is paused when the widget is up)
}