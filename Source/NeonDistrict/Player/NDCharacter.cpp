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
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"

namespace
{
	const TCHAR* CubeMeshPath = TEXT("/Engine/BasicShapes/Cube.Cube");
	const TCHAR* SphereMeshPath = TEXT("/Engine/BasicShapes/Sphere.Sphere");
	const TCHAR* BasicShapeMatPath = TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial");
	// Engine-cooked humanoid mannequin (head, torso, limbs, face) + walk anims.
	const TCHAR* MannequinMeshPath = TEXT("/Engine/Tutorial/SubEditors/TutorialAssets/Character/TutorialTPP.TutorialTPP");
	const TCHAR* MannequinAnimBPPath = TEXT("/Engine/Tutorial/SubEditors/TutorialAssets/Character/TutorialTPP_AnimBlueprint.TutorialTPP_AnimBlueprint_C");
	const TCHAR* MannequinMatPath = TEXT("/Engine/Tutorial/SubEditors/TutorialAssets/Character/TutorialTPP_Mat.TutorialTPP_Mat");

	void ApplyTint(UStaticMeshComponent* Mesh, UObject* Owner, const FLinearColor& Color, float Emissive = 0.0f)
	{
		if (!Mesh)
		{
			return;
		}
		if (UMaterialInterface* EngineMat = LoadObject<UMaterialInterface>(nullptr, BasicShapeMatPath))
		{
			UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(EngineMat, Owner);
			MID->SetVectorParameterValue(TEXT("Color"), Color);
			MID->SetVectorParameterValue(TEXT("BaseColor"), Color);
			MID->SetVectorParameterValue(TEXT("NeonColor"), Color);
			MID->SetScalarParameterValue(TEXT("EmissiveStrength"), Emissive);
			Mesh->SetMaterial(0, MID);
		}
	}
}

ANDCharacter::ANDCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// --- Player visual: engine mannequin (human) with primitive fallback ---
	// ACharacter ships an empty SkeletalMesh; try to load the engine-cooked
	// Tutorial mannequin so the player reads as a real humanoid with a face.
	// If it cannot load (packaged edge case), fall back to the segmented
	// primitive proxy so the benchmark never loses the visible pawn.
	USkeletalMeshComponent* Skel = GetMesh();
	bool bMannequinLoaded = false;
	if (Skel)
	{
		Skel->SetGenerateOverlapEvents(false);
		Skel->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		if (USkeletalMesh* Mannequin = LoadObject<USkeletalMesh>(nullptr, MannequinMeshPath))
		{
			Skel->SetSkeletalMesh(Mannequin);
			Skel->SetVisibility(true);
			// Mannequin root sits at its feet; capsule origin is at its center.
			Skel->SetRelativeLocation(FVector(0, 0, -88.0f));
			Skel->SetRelativeScale3D(FVector(0.98f, 0.98f, 0.98f));
			if (UClass* ABP = LoadObject<UClass>(nullptr, MannequinAnimBPPath))
			{
				Skel->SetAnimInstanceClass(ABP);
			}
			if (UMaterialInterface* Mat = LoadObject<UMaterialInterface>(nullptr, MannequinMatPath))
			{
				UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(Mat, this);
				MID->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.10f, 0.13f, 0.30f));
				Skel->SetMaterial(0, MID);
			}
			bMannequinLoaded = true;
		}
		else
		{
			Skel->SetVisibility(false);
		}
	}

	// Primitive proxy parts (hidden when the mannequin loads; kept as fallback).
	PlayerBody = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlayerBody"));
	PlayerBody->SetupAttachment(GetRootComponent());
	PlayerBody->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PlayerBody->bCastDynamicShadow = true;
	PlayerBody->SetRelativeLocation(FVector(0, 0, 16.0f));
	PlayerBody->SetRelativeScale3D(FVector(0.56f, 0.36f, 0.64f));

	PlayerHead = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlayerHead"));
	PlayerHead->SetupAttachment(GetRootComponent());
	PlayerHead->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PlayerHead->SetRelativeLocation(FVector(0, 0, 106.0f));
	PlayerHead->SetRelativeScale3D(FVector(0.39f, 0.35f, 0.41f));

	PlayerHair = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlayerHair"));
	PlayerHair->SetupAttachment(GetRootComponent());
	PlayerHair->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PlayerHair->SetRelativeLocation(FVector(0, -3.0f, 131.0f));
	PlayerHair->SetRelativeScale3D(FVector(0.42f, 0.36f, 0.13f));

	PlayerJacket = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlayerJacket"));
	PlayerJacket->SetupAttachment(GetRootComponent());
	PlayerJacket->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PlayerJacket->SetRelativeLocation(FVector(2, -28, 20.0f));
	PlayerJacket->SetRelativeScale3D(FVector(0.42f, 0.08f, 0.56f));

	PlayerLeftArm = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlayerLeftArm"));
	PlayerLeftArm->SetupAttachment(GetRootComponent());
	PlayerLeftArm->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PlayerLeftArm->SetRelativeLocation(FVector(0, -48.0f, 22.0f));
	PlayerLeftArm->SetRelativeScale3D(FVector(0.19f, 0.18f, 0.52f));

	PlayerRightArm = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlayerRightArm"));
	PlayerRightArm->SetupAttachment(GetRootComponent());
	PlayerRightArm->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PlayerRightArm->SetRelativeLocation(FVector(0, 48.0f, 22.0f));
	PlayerRightArm->SetRelativeScale3D(FVector(0.19f, 0.18f, 0.52f));

	PlayerLeftHand = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlayerLeftHand"));
	PlayerLeftHand->SetupAttachment(GetRootComponent());
	PlayerLeftHand->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PlayerLeftHand->SetRelativeLocation(FVector(0, -48.0f, -18.0f));
	PlayerLeftHand->SetRelativeScale3D(FVector(0.17f, 0.16f, 0.17f));

	PlayerRightHand = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlayerRightHand"));
	PlayerRightHand->SetupAttachment(GetRootComponent());
	PlayerRightHand->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PlayerRightHand->SetRelativeLocation(FVector(0, 48.0f, -18.0f));
	PlayerRightHand->SetRelativeScale3D(FVector(0.17f, 0.16f, 0.17f));

	PlayerLeftLeg = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlayerLeftLeg"));
	PlayerLeftLeg->SetupAttachment(GetRootComponent());
	PlayerLeftLeg->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PlayerLeftLeg->SetRelativeLocation(FVector(0, -18.0f, -58.0f));
	PlayerLeftLeg->SetRelativeScale3D(FVector(0.22f, 0.17f, 0.64f));

	PlayerRightLeg = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlayerRightLeg"));
	PlayerRightLeg->SetupAttachment(GetRootComponent());
	PlayerRightLeg->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PlayerRightLeg->SetRelativeLocation(FVector(0, 18.0f, -58.0f));
	PlayerRightLeg->SetRelativeScale3D(FVector(0.22f, 0.17f, 0.64f));

	PlayerLeftFoot = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlayerLeftFoot"));
	PlayerLeftFoot->SetupAttachment(GetRootComponent());
	PlayerLeftFoot->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PlayerLeftFoot->SetRelativeLocation(FVector(20.0f, -18.0f, -124.0f));
	PlayerLeftFoot->SetRelativeScale3D(FVector(0.30f, 0.19f, 0.10f));

	PlayerRightFoot = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlayerRightFoot"));
	PlayerRightFoot->SetupAttachment(GetRootComponent());
	PlayerRightFoot->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PlayerRightFoot->SetRelativeLocation(FVector(20.0f, 18.0f, -124.0f));
	PlayerRightFoot->SetRelativeScale3D(FVector(0.30f, 0.19f, 0.10f));

	WeaponBody = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponBody"));
	WeaponBody->SetupAttachment(GetRootComponent());
	WeaponBody->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponBody->SetRelativeLocation(FVector(42.0f, 48.0f, -8.0f));
	WeaponBody->SetRelativeRotation(FRotator(0.0f, 8.0f, -12.0f));
	WeaponBody->SetRelativeScale3D(FVector(0.48f, 0.09f, 0.10f));

	WeaponBarrel = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponBarrel"));
	WeaponBarrel->SetupAttachment(GetRootComponent());
	WeaponBarrel->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponBarrel->SetRelativeLocation(FVector(76.0f, 48.0f, -4.0f));
	WeaponBarrel->SetRelativeRotation(FRotator(0.0f, 8.0f, -12.0f));
	WeaponBarrel->SetRelativeScale3D(FVector(0.28f, 0.045f, 0.045f));

	WeaponCore = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponCore"));
	WeaponCore->SetupAttachment(GetRootComponent());
	WeaponCore->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponCore->SetRelativeLocation(FVector(50.0f, 48.0f, 4.0f));
	WeaponCore->SetRelativeScale3D(FVector(0.10f));

	if (UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, CubeMeshPath))
	{
		PlayerBody->SetStaticMesh(Cube);
		PlayerHair->SetStaticMesh(Cube);
		PlayerJacket->SetStaticMesh(Cube);
		PlayerLeftArm->SetStaticMesh(Cube);
		PlayerRightArm->SetStaticMesh(Cube);
		PlayerLeftLeg->SetStaticMesh(Cube);
		PlayerRightLeg->SetStaticMesh(Cube);
		PlayerLeftFoot->SetStaticMesh(Cube);
		PlayerRightFoot->SetStaticMesh(Cube);
		WeaponBody->SetStaticMesh(Cube);
		WeaponBarrel->SetStaticMesh(Cube);
	}
	if (UStaticMesh* Sphere = LoadObject<UStaticMesh>(nullptr, SphereMeshPath))
	{
		PlayerHead->SetStaticMesh(Sphere);
		PlayerLeftHand->SetStaticMesh(Sphere);
		PlayerRightHand->SetStaticMesh(Sphere);
		WeaponCore->SetStaticMesh(Sphere);
	}
	ApplyTint(PlayerBody, this, FLinearColor(0.08f, 0.10f, 0.22f), 0.0f);
	ApplyTint(PlayerJacket, this, FLinearColor(0.95f, 0.12f, 0.62f), 2.0f);
	ApplyTint(PlayerHead, this, FLinearColor(0.82f, 0.56f, 0.38f), 0.0f);
	ApplyTint(PlayerHair, this, FLinearColor(0.02f, 0.015f, 0.018f), 0.0f);
	ApplyTint(PlayerLeftArm, this, FLinearColor(0.12f, 0.16f, 0.30f), 0.0f);
	ApplyTint(PlayerRightArm, this, FLinearColor(0.12f, 0.16f, 0.30f), 0.0f);
	ApplyTint(PlayerLeftHand, this, FLinearColor(0.82f, 0.56f, 0.38f), 0.0f);
	ApplyTint(PlayerRightHand, this, FLinearColor(0.82f, 0.56f, 0.38f), 0.0f);
	ApplyTint(PlayerLeftLeg, this, FLinearColor(0.03f, 0.04f, 0.08f), 0.0f);
	ApplyTint(PlayerRightLeg, this, FLinearColor(0.03f, 0.04f, 0.08f), 0.0f);
	ApplyTint(PlayerLeftFoot, this, FLinearColor(0.005f, 0.006f, 0.01f), 0.0f);
	ApplyTint(PlayerRightFoot, this, FLinearColor(0.005f, 0.006f, 0.01f), 0.0f);
	ApplyTint(WeaponBody, this, FLinearColor(0.025f, 0.030f, 0.040f), 0.0f);
	ApplyTint(WeaponBarrel, this, FLinearColor(0.04f, 0.75f, 1.0f), 2.0f);
	ApplyTint(WeaponCore, this, FLinearColor(0.12f, 0.90f, 1.0f), 5.0f);
	SetWeaponVisible(false);

	// Hide the primitive proxy when the mannequin is showing.
	const bool bShowProxy = !bMannequinLoaded;
	PlayerBody->SetVisibility(bShowProxy);
	PlayerHead->SetVisibility(bShowProxy);
	PlayerHair->SetVisibility(bShowProxy);
	PlayerJacket->SetVisibility(bShowProxy);
	PlayerLeftArm->SetVisibility(bShowProxy);
	PlayerRightArm->SetVisibility(bShowProxy);
	PlayerLeftHand->SetVisibility(bShowProxy);
	PlayerRightHand->SetVisibility(bShowProxy);
	PlayerLeftLeg->SetVisibility(bShowProxy);
	PlayerRightLeg->SetVisibility(bShowProxy);
	PlayerLeftFoot->SetVisibility(bShowProxy);
	PlayerRightFoot->SetVisibility(bShowProxy);

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

void ANDCharacter::SetWeaponVisible(bool bVisible)
{
	if (WeaponBody) { WeaponBody->SetVisibility(bVisible); }
	if (WeaponBarrel) { WeaponBarrel->SetVisibility(bVisible); }
	if (WeaponCore) { WeaponCore->SetVisibility(bVisible); }
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
