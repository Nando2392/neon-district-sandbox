// Copyright Neon District Sandbox. Public benchmark repo — original content only.

#include "Player/NDPlayerController.h"
#include "UI/NDMainMenuWidget.h"
#include "Player/NDCharacter.h"
#include "Player/NDInteractable.h"
#include "Vehicle/NDVehicle.h"
#include "UI/NDHUDWidget.h"
#include "UI/NDPauseWidget.h"
#include "Core/NDGameInstance.h"
#include "Core/NDPerfConstants.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Components/InputComponent.h"

ANDPlayerController::ANDPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ANDPlayerController::BeginPlay()
{
	Super::BeginPlay();

	const FString LevelName = GetWorld() ? GetWorld()->GetName() : FString();

	// Main menu: show the procedural menu instead of the HUD (no gameplay).
	if (LevelName == TEXT("ND_MainMenu"))
	{
		UNDMainMenuWidget* Menu = CreateWidget<UNDMainMenuWidget>(this, UNDMainMenuWidget::StaticClass());
		if (Menu)
		{
			Menu->AddToViewport(100);
		}
		SetInputMode(FInputModeUIOnly());
		bShowMouseCursor = true;
		return;
	}

	CreateInputActions();

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(InputContext, 0);
	}

	// HUD (code-created; replaceable with a BP subclass in the editor).
	if (!HUDWidget)
	{
		HUDWidget = CreateWidget<UNDHUDWidget>(this, UNDHUDWidget::StaticClass());
		if (HUDWidget)
		{
			HUDWidget->AddToViewport(0);
		}
	}

	// Keep a direct character ref (spawned by the game mode).
	PlayerCharacter = Cast<ANDCharacter>(GetPawn());

	// Restore a pending save location if the game instance asked for it.
	if (UNDGameInstance* GI = GetGameInstance<UNDGameInstance>())
	{
		GI->CachePendingPlayer(this);
	}
}

void ANDPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bIsDriving)
	{
		UpdateInteractionTarget();
	}
}

void ANDPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
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
	}
}

void ANDPlayerController::CreateInputActions()
{
	InputContext = NewObject<UInputMappingContext>(this, TEXT("ND_DefaultContext"));

	auto MakeAction = [this](const TCHAR* Name, EInputActionValueType ValueType)
	{
		UInputAction* Action = NewObject<UInputAction>(this, Name);
		Action->ValueType = ValueType;
		return Action;
	};

	IA_Move = MakeAction(TEXT("IA_Move"), EInputActionValueType::Axis2D);
	IA_Look = MakeAction(TEXT("IA_Look"), EInputActionValueType::Axis2D);
	IA_Jump = MakeAction(TEXT("IA_Jump"), EInputActionValueType::Boolean);
	IA_Sprint = MakeAction(TEXT("IA_Sprint"), EInputActionValueType::Boolean);
	IA_Interact = MakeAction(TEXT("IA_Interact"), EInputActionValueType::Boolean);
	IA_Vehicle = MakeAction(TEXT("IA_Vehicle"), EInputActionValueType::Boolean);
	IA_Pause = MakeAction(TEXT("IA_Pause"), EInputActionValueType::Boolean);
	IA_QuickSave = MakeAction(TEXT("IA_QuickSave"), EInputActionValueType::Boolean);
	IA_QuickLoad = MakeAction(TEXT("IA_QuickLoad"), EInputActionValueType::Boolean);

	InputContext->MapKey(IA_Move, EKeys::W);
	InputContext->MapKey(IA_Move, EKeys::S);
	InputContext->MapKey(IA_Move, EKeys::A);
	InputContext->MapKey(IA_Move, EKeys::D);
	InputContext->MapKey(IA_Move, EKeys::Up);
	InputContext->MapKey(IA_Move, EKeys::Down);
	InputContext->MapKey(IA_Move, EKeys::Left);
	InputContext->MapKey(IA_Move, EKeys::Right);

	InputContext->MapKey(IA_Look, EKeys::MouseX);
	InputContext->MapKey(IA_Look, EKeys::MouseY);

	InputContext->MapKey(IA_Jump, EKeys::SpaceBar);
	InputContext->MapKey(IA_Sprint, EKeys::LeftShift);
	InputContext->MapKey(IA_Interact, EKeys::E);
	InputContext->MapKey(IA_Vehicle, EKeys::F);
	InputContext->MapKey(IA_Pause, EKeys::Escape);
	InputContext->MapKey(IA_QuickSave, EKeys::F5);
	InputContext->MapKey(IA_QuickLoad, EKeys::F9);
}

void ANDPlayerController::HandleMove(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();

	if (bIsDriving && DrivenVehicle)
	{
		// W/S = throttle/brake, A/D = steering (fed to the Chaos component).
		DrivenVehicle->ApplyDriveInput(Axis.Y, Axis.X);
		return;
	}

	if (APawn* Pawn = GetPawn())
	{
		const FRotator Yaw(0.0f, GetControlRotation().Yaw, 0.0f);
		const FVector Forward = FRotationMatrix(Yaw).GetUnitAxis(EAxis::X);
		const FVector Right = FRotationMatrix(Yaw).GetUnitAxis(EAxis::Y);
		Pawn->AddMovementInput(Forward, Axis.X);
		Pawn->AddMovementInput(Right, Axis.Y);
	}
}

void ANDPlayerController::HandleLook(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	AddYawInput(Axis.X);
	AddPitchInput(-Axis.Y);
}

void ANDPlayerController::HandleJumpStart()
{
	if (bIsDriving)
	{
		if (DrivenVehicle)
		{
			DrivenVehicle->SetHandbrake(true); // Space = handbrake while driving
		}
		return;
	}
	if (PlayerCharacter)
	{
		PlayerCharacter->Jump();
	}
}

void ANDPlayerController::HandleJumpStop()
{
	if (bIsDriving)
	{
		if (DrivenVehicle)
		{
			DrivenVehicle->SetHandbrake(false);
		}
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
	if (bIsDriving || !CurrentInteractable)
	{
		return;
	}
	if (CurrentInteractable->Implements<UNDInteractable>())
	{
		INDIInteractable::Execute_Interact(CurrentInteractable, this);
	}
}

void ANDPlayerController::HandleEnterExitVehicle()
{
	if (bIsDriving)
	{
		if (DrivenVehicle)
		{
			DrivenVehicle->ExitVehicle(this);
		}
		return;
	}

	if (CurrentInteractable && CurrentInteractable->Implements<UNDInteractable>())
	{
		// Let the vehicle handle entering itself; other interactables ignore F.
		INDIInteractable::Execute_Interact(CurrentInteractable, this);
	}
}

void ANDPlayerController::HandlePause()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (World->IsPaused())
	{
		World->SetPaused(false);
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
		World->SetPaused(true);
		PauseWidget = CreateWidget<UNDPauseWidget>(this, UNDPauseWidget::StaticClass());
		if (PauseWidget)
		{
			PauseWidget->AddToViewport(10);
		}
		SetInputMode(FInputModeGameAndUI());
		bShowMouseCursor = true;
	}
}

void ANDPlayerController::HandlePauseFromWidget()
{
	HandlePause(); // resume (world is paused when the widget is up)
}

void ANDPlayerController::HandleQuickSave()
{
	if (UNDGameInstance* GI = GetGameInstance<UNDGameInstance>())
	{
		GI->SaveGame();
		if (HUDWidget)
		{
			HUDWidget->ShowNotification(FText::FromString(TEXT("Progreso guardado (F5)")));
		}
	}
}

void ANDPlayerController::HandleQuickLoad()
{
	if (UNDGameInstance* GI = GetGameInstance<UNDGameInstance>())
	{
		if (GI->LoadGame())
		{
			// Teleport the character to the saved location (mission state already restored).
			if (ANDCharacter* Pawn = Cast<ANDCharacter>(GetPawn()))
			{
				const FVector SavedLoc = GI->GetMutableSave()->PlayerLocation;
				if (!SavedLoc.IsNearlyZero())
				{
					Pawn->SetActorLocation(SavedLoc);
					Pawn->SetActorRotation(FRotator(0.0f, GI->GetMutableSave()->PlayerYaw, 0.0f));
				}
			}
			if (HUDWidget)
			{
				HUDWidget->ShowNotification(FText::FromString(TEXT("Partida cargada (F9)")));
			}
		}
		else if (HUDWidget)
		{
			HUDWidget->ShowNotification(FText::FromString(TEXT("No hay partida guardada")));
		}
	}
}

void ANDPlayerController::UpdateInteractionTarget()
{
	AActor* NewTarget = nullptr;

	// Probe from the camera forward so the prompt matches what the player sees.
	FVector ViewLocation;
	FRotator ViewRotation;
	GetPlayerViewPoint(ViewLocation, ViewRotation);

	FCollisionQueryParams Params(SCENE_QUERY_STAT(NDInteractionProbe), true);
	if (APawn* Pawn = GetPawn())
	{
		Params.AddIgnoredActor(Pawn);
	}

	FHitResult Hit;
	if (GetWorld()->LineTraceSingleByChannel(Hit, ViewLocation,
		ViewLocation + ViewRotation.Vector() * NDPerf::InteractionRange,
		ECC_Visibility, Params))
	{
		AActor* HitActor = Hit.GetActor();
		if (HitActor && HitActor->Implements<UNDInteractable>())
		{
			NewTarget = HitActor;
		}
	}

	if (NewTarget != CurrentInteractable)
	{
		CurrentInteractable = NewTarget;
		if (HUDWidget)
		{
			FText Prompt = FText::GetEmpty();
			if (CurrentInteractable)
			{
				Prompt = INDIInteractable::Execute_GetInteractionPrompt(CurrentInteractable);
			}
			HUDWidget->SetInteractionPrompt(Prompt);
		}
	}
}

void ANDPlayerController::SetDrivingState(ANDVehicle* Vehicle, bool bEntering)
{
	DrivenVehicle = bEntering ? Vehicle : nullptr;
	bIsDriving = bEntering;

	if (bEntering && PlayerCharacter)
	{
		// Hide the character while driving (no physics under the vehicle).
		PlayerCharacter->SetActorHiddenInGame(true);
		PlayerCharacter->SetActorEnableCollision(false);
		PlayerCharacter->SetActorTickEnabled(false);
	}

	if (HUDWidget)
	{
		HUDWidget->SetVehicleState(bEntering, bEntering ? Vehicle->GetDisplayName() : FText::GetEmpty());
	}

	if (bEntering)
	{
		if (HUDWidget)
		{
			HUDWidget->SetInteractionPrompt(FText::GetEmpty());
		}
	}
	else
	{
		RestoreCharacterFromVehicle();
	}
}

void ANDPlayerController::RestoreCharacterFromVehicle()
{
	if (PlayerCharacter)
	{
		PlayerCharacter->SetActorHiddenInGame(false);
		PlayerCharacter->SetActorEnableCollision(true);
		PlayerCharacter->SetActorTickEnabled(true);
		Possess(PlayerCharacter);
		// Park the character beside the door when leaving.
		if (DrivenVehicle)
		{
			const FVector ExitLoc = DrivenVehicle->GetActorLocation()
				- DrivenVehicle->GetActorForwardVector() * 220.0f;
			PlayerCharacter->SetActorLocation(ExitLoc + FVector(0.0f, 0.0f, 90.0f));
		}
	}
	DrivenVehicle = nullptr;
	bIsDriving = false;
}
