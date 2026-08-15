// Copyright Neon District Sandbox. Public benchmark repo — original content only.

#include "Player/NDCharacter.h"
#include "Core/NDPerfConstants.h"
#include "Core/NDGameInstance.h"
#include "Audio/NDAudioManager.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/InputComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"

ANDCharacter::ANDCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// --- Player visual proxy (zero asset dependency) ---
	// ACharacter ships an empty SkeletalMesh; hide it and attach a Cube-based
	// body mesh so the player is always visible even before any ABP/import.
	USkeletalMeshComponent* Skel = GetMesh();
	if (Skel)
	{
		Skel->SetVisibility(false);
		Skel->SetGenerateOverlapEvents(false);
	}

	PlayerBody = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlayerBody"));
	PlayerBody->SetupAttachment(GetRootComponent());
	PlayerBody->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PlayerBody->bCastDynamicShadow = false;
	PlayerBody->SetRelativeLocation(FVector(0, 0, -88.0f)); // align with capsule standing height
	PlayerBody->SetRelativeScale3D(FVector(0.45f, 0.45f, 1.7f)); // ~slim humanoid

	if (UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")))
	{
		PlayerBody->SetStaticMesh(Cube);
		// Neon-accented material tint.
		if (UMaterialInterface* EngineMat = LoadObject<UMaterialInterface>(
			nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial")))
		{
			UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(EngineMat, this);
			if (MID)
			{
				MID->SetVectorParameterValue(TEXT("NeonColor"), FLinearColor(0.9f, 0.3f, 1.0f)); // violet-pink
				MID->SetScalarParameterValue(TEXT("EmissiveStrength"), 2.0f);
				PlayerBody->SetMaterial(0, MID);
			}
		}
	}

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(GetRootComponent());
	SpringArm->TargetArmLength = NDPerf::CameraSpringArmLength;
	SpringArm->bUsePawnControlRotation = true;
	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagSpeed = 8.0f;
	SpringArm->bDoCollisionTest = true;
	SpringArm->ProbeSize = NDPerf::CameraCollisionProbeSize;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = NDPerf::WalkSpeed;
		MoveComp->MaxWalkSpeedCrouched = NDPerf::WalkSpeed * 0.5f;
		MoveComp->JumpZVelocity = NDPerf::JumpVelocity;
		MoveComp->AirControl = 0.35f;
		MoveComp->bOrientRotationToMovement = true;
		MoveComp->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	}
}

void ANDCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	AccumulateFootstepDistance(DeltaSeconds);
}

void ANDCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	// Movement is driven entirely from the controller's Enhanced Input actions.
}

void ANDCharacter::SetSprinting(bool bNewSprinting)
{
	this->bSprinting = bNewSprinting;
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = bNewSprinting ? NDPerf::SprintSpeed : NDPerf::RunSpeed;
	}
}

void ANDCharacter::AccumulateFootstepDistance(float DeltaSeconds)
{
	const float Speed = GetVelocity().Size2D();
	if (Speed < 40.0f)
	{
		return; // idle/air: no steps
	}

	WalkedDistance += Speed * DeltaSeconds;
	const float Interval = bSprinting ? FootstepIntervalCm * 1.5f : FootstepIntervalCm;
	if (WalkedDistance >= Interval)
	{
		WalkedDistance = 0.0f;
		PlayFootstep();
	}
}

void ANDCharacter::PlayFootstep()
{
	if (UNDGameInstance* GI = GetGameInstance<UNDGameInstance>())
	{
		if (UNDAudioManager* Audio = GI->GetAudioManager())
		{
			Audio->PlayFootstep(this);
		}
	}
}
