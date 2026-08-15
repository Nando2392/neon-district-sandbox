// Copyright Neon District Sandbox. Public benchmark repo — original content only

#include "Player/NDPlayerController.h"
#include "Player/NDCharacter.h"
#include "Vehicle/NDVehicle.h"
#include "UI/NDHUDWidget.h"
#include "Blueprint/UserWidget.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "Engine/World.h"
#include "Components/InputComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

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
		return;
	}

	// Gameplay level: setup input
	SetupGameplayInput();
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
	// Simplified - no actual interaction for benchmark
}

void ANDPlayerController::HandleEnterExitVehicle()
{
	// Simplified - no vehicle enter/exit for benchmark
}

void ANDPlayerController::HandleQuickSave()
{
	// Simplified - no save for benchmark
}

void ANDPlayerController::HandleQuickLoad()
{
	// Simplified - no load for benchmark
}

void ANDPlayerController::UpdateInteractionTarget()
{
	CurrentInteractable = nullptr;
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
		SetInputMode(FInputModeGameAndUI());
		bShowMouseCursor = true;
	}
}

void ANDPlayerController::HandlePauseFromWidget()
{
	HandlePause(); // resume (world is paused when the widget is up)
}