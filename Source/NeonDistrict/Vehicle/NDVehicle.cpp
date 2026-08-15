// Copyright Neon District Sandbox. Public benchmark repo — original content only.

#include "Vehicle/NDVehicle.h"
#include "Player/NDPlayerController.h"
#include "Core/NDPerfConstants.h"
#include "Core/NDGameInstance.h"
#include "Audio/NDAudioManager.h"

#include "ChaosWheeledVehicleMovementComponent.h"
#include "ChaosVehicleWheel.h"
#include "Components/StaticMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "FX/NDVFXManager.h"

void ANDVehicle::NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp,
	bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);

	const float Now = GetWorld()->GetTimeSeconds();
	if (Now - LastImpactTime < ImpactCooldown)
	{
		return; // gate: at most one impact event per 0.6s
	}
	LastImpactTime = Now;

	if (NormalImpulse.Size() < 12000.0f)
	{
		return; // gentle touch: no spark spam
	}

	if (UNDGameInstance* GI = GetGameInstance<UNDGameInstance>())
	{
		if (UNDVFXManager* FX = GI->GetVFXManager())
		{
			FX->SpawnFX(ENDFXType::ImpactSparks, HitLocation, HitNormal.Rotation());
		}
		if (UNDAudioManager* Audio = GI->GetAudioManager())
		{
			Audio->PlayImpact(this);
		}
	}
}

ANDVehicle::ANDVehicle()
{
	PrimaryActorTick.bCanEverTick = true;

	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
	SetRootComponent(BodyMesh);
	BodyMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BodyMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);

	VehicleMovement = CreateDefaultSubobject<UChaosWheeledVehicleMovementComponent>(TEXT("VehicleMovement"));
	VehicleMovement->SetUpdatedComponent(BodyMesh);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(BodyMesh);
	SpringArm->TargetArmLength = 520.0f;
	SpringArm->bUsePawnControlRotation = false;
	SpringArm->SetRelativeRotation(FRotator(-8.0f, 0.0f, 0.0f));

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
}

void ANDVehicle::BeginPlay()
{
	Super::BeginPlay();
	EnsureWheels();
}

void ANDVehicle::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bBeingDriven && VehicleMovement)
	{
		if (UNDGameInstance* GI = GetGameInstance<UNDGameInstance>())
		{
			if (UNDAudioManager* Audio = GI->GetAudioManager())
			{
				Audio->UpdateEngineSound(this, GetForwardSpeedKmh());
			}
		}
	}
}

void ANDVehicle::ApplyDriveInput(float ThrottleBrake, float Steering)
{
	if (!VehicleMovement)
	{
		return;
	}
	VehicleMovement->SetThrottleInput(FMath::Clamp(ThrottleBrake, -1.0f, 1.0f));
	VehicleMovement->SetSteeringInput(FMath::Clamp(Steering, -1.0f, 1.0f));
}

void ANDVehicle::SetHandbrake(bool bEngaged)
{
	if (VehicleMovement)
	{
		VehicleMovement->SetBrakeInput(bEngaged ? 1.0f : 0.0f);
	}
}

void ANDVehicle::EnterVehicle(APlayerController* PC)
{
	if (!PC || bBeingDriven)
	{
		return;
	}

	bBeingDriven = true;

	if (ANDPlayerController* NDPC = Cast<ANDPlayerController>(PC))
	{
		NDPC->SetDrivingState(this, true);
	}

	PC->Possess(this);
	PC->SetControlRotation(GetActorRotation());

	if (UNDGameInstance* GI = GetGameInstance<UNDGameInstance>())
	{
		if (UNDAudioManager* Audio = GI->GetAudioManager())
		{
			Audio->StartEngineLoop(this);
		}
	}
}

void ANDVehicle::ExitVehicle(APlayerController* PC)
{
	if (!bBeingDriven)
	{
		return;
	}

	bBeingDriven = false;

	if (UNDGameInstance* GI = GetGameInstance<UNDGameInstance>())
	{
		if (UNDAudioManager* Audio = GI->GetAudioManager())
		{
			Audio->StopEngineLoop();
		}
	}

	if (ANDPlayerController* NDPC = Cast<ANDPlayerController>(PC))
	{
		NDPC->SetDrivingState(this, false); // restores + repossesses the character
	}
}

FText ANDVehicle::GetInteractionPrompt_Implementation() const
{
	return FText::FromString(TEXT("Entrar al vehículo (E)"));
}

bool ANDVehicle::Interact_Implementation(APlayerController* PlayerController)
{
	EnterVehicle(PlayerController);
	return true;
}

float ANDVehicle::GetForwardSpeedKmh() const
{
	return VehicleMovement ? VehicleMovement->GetForwardSpeed() * 0.036f : 0.0f;
}

void ANDVehicle::EnsureWheels()
{
	if (!VehicleMovement || VehicleMovement->WheelSetups.Num() > 0)
	{
		return;
	}

	// Minimal code-only wheel setup so the vehicle drives before editor tuning.
	FChaosWheelSetup Wheel;
	Wheel.WheelClass = UChaosVehicleWheel::StaticClass();

	auto AddWheel = [&](float X, float Y)
	{
		FChaosWheelSetup W = Wheel;
		W.AdditionalOffset = FVector(X, Y, -35.0f);
		VehicleMovement->WheelSetups.Add(W);
	};

	AddWheel(145.0f,  92.0f); // front-left
	AddWheel(145.0f, -92.0f); // front-right
	AddWheel(-150.0f,  92.0f); // rear-left
	AddWheel(-150.0f, -92.0f); // rear-right

	// Sensible defaults for a small urban coupe.
	VehicleMovement->Mass = 1300.0f;
	VehicleMovement->EngineSetup.MaxRPM = 6800.0f;
	VehicleMovement->EngineSetup.EngineRevUpMOI = 0.2f;
	VehicleMovement->TransmissionSetup.FinalRatio = 3.2f;
	VehicleMovement->SteeringSetup.SteeringCurve.GetRichCurve()->Reset();
	VehicleMovement->SteeringSetup.SteeringCurve.GetRichCurve()->AddKey(0.0f, 0.9f);
	VehicleMovement->SteeringSetup.SteeringCurve.GetRichCurve()->AddKey(1800.0f, 0.5f);
	VehicleMovement->SteeringSetup.SteeringCurve.GetRichCurve()->AddKey(3600.0f, 0.25f);

	VehicleMovement->UpdatedComponent = BodyMesh;
	VehicleMovement->RecreatePhysicsState();
}
