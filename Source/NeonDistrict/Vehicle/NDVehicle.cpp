// Copyright Neon District Sandbox. Public benchmark repo — original content only.

#include "Vehicle/NDVehicle.h"
#include "Player/NDPlayerController.h"
#include "Core/NDPerfConstants.h"
#include "Core/NDGameInstance.h"
#include "Audio/NDAudioManager.h"

#include "ChaosWheeledVehicleMovementComponent.h"
#include "ChaosVehicleWheel.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "ProceduralMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "FX/NDVFXManager.h"

namespace
{
	const TCHAR* CubeMeshPath = TEXT("/Engine/BasicShapes/Cube.Cube");
	const TCHAR* CylinderMeshPath = TEXT("/Engine/BasicShapes/Cylinder.Cylinder");
	const TCHAR* ConeMeshPath = TEXT("/Engine/BasicShapes/Cone.Cone");
	const TCHAR* BasicShapeMatPath = TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial");

	void TintVehiclePart(UStaticMeshComponent* Mesh, UObject* Owner, const FLinearColor& Color, float Emissive = 0.0f)
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

	UMaterialInstanceDynamic* MakeRuntimeVehicleMaterial(UObject* Owner, const FLinearColor& Color, float Emissive = 0.0f)
	{
		if (UMaterialInterface* EngineMat = LoadObject<UMaterialInterface>(nullptr, BasicShapeMatPath))
		{
			UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(EngineMat, Owner);
			MID->SetVectorParameterValue(TEXT("Color"), Color);
			MID->SetVectorParameterValue(TEXT("BaseColor"), Color);
			MID->SetVectorParameterValue(TEXT("NeonColor"), Color);
			MID->SetScalarParameterValue(TEXT("EmissiveStrength"), Emissive);
			return MID;
		}
		return nullptr;
	}

	void ApplyAuthoredCoupeMaterials(UStaticMeshComponent* Mesh, UObject* Owner)
	{
		if (!Mesh || !Mesh->GetStaticMesh())
		{
			return;
		}

		const TArray<FStaticMaterial>& Slots = Mesh->GetStaticMesh()->GetStaticMaterials();
		for (int32 Index = 0; Index < Mesh->GetNumMaterials(); ++Index)
		{
			const FString SlotName = Slots.IsValidIndex(Index) ? Slots[Index].MaterialSlotName.ToString() : FString();
			FLinearColor Color(0.006f, 0.026f, 0.075f);
			float Emissive = 0.0f;

			if (SlotName.Contains(TEXT("Glass")))
			{
				Color = FLinearColor(0.002f, 0.008f, 0.018f);
			}
			else if (SlotName.Contains(TEXT("Tire")) || SlotName.Contains(TEXT("Technical")))
			{
				Color = FLinearColor(0.004f, 0.004f, 0.006f);
			}
			else if (SlotName.Contains(TEXT("Rim")))
			{
				Color = FLinearColor(0.28f, 0.30f, 0.34f);
			}
			else if (SlotName.Contains(TEXT("Brake")))
			{
				Color = FLinearColor(0.50f, 0.02f, 0.015f);
			}
			else if (SlotName.Contains(TEXT("FrontLight")))
			{
				Color = FLinearColor(0.78f, 0.92f, 1.0f);
				Emissive = 4.0f;
			}
			else if (SlotName.Contains(TEXT("RearLight")))
			{
				Color = FLinearColor(1.0f, 0.025f, 0.035f);
				Emissive = 4.5f;
			}

			if (UMaterialInstanceDynamic* MID = MakeRuntimeVehicleMaterial(Owner, Color, Emissive))
			{
				Mesh->SetMaterial(Index, MID);
			}
		}
	}
}

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
	// Low, wide, multi-part supercar silhouette with zero external assets.
	if (UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, CubeMeshPath))
	{
		BodyMesh->SetStaticMesh(Cube);
		BodyMesh->SetRelativeScale3D(FVector(3.6f, 1.76f, 0.38f));
	}
	TintVehiclePart(BodyMesh, this, FLinearColor(0.02f, 0.26f, 0.95f), 1.2f);
	BodyMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BodyMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	// This remains the deliberately simple, stable Chaos collider. The visible
	// body is an original procedural mesh below, so collision and art can evolve
	// independently without destabilizing wheels or driving input.
	BodyMesh->SetVisibility(true, false);
	BodyMesh->bRenderInMainPass = false;
	BodyMesh->bRenderInDepthPass = false;
	BodyMesh->bCastHiddenShadow = false;

	// Chaos scales BodyMesh non-uniformly for the stable collision chassis.
	// This visual root inherits its movement/rotation but cancels that scale, so
	// art is neither flattened nor left behind when physics updates the body.
	VehicleRoot = CreateDefaultSubobject<USceneComponent>(TEXT("VehicleRoot"));
	VehicleRoot->SetupAttachment(BodyMesh);
	VehicleRoot->SetRelativeScale3D(FVector(1.0f / 3.6f, 1.0f / 1.76f, 1.0f / 0.38f));

	AuthoredBodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AuthoredBodyMesh"));
	AuthoredBodyMesh->SetupAttachment(VehicleRoot);
	AuthoredBodyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AuthoredBodyMesh->SetCastShadow(true);
	AuthoredBodyMesh->bCastDynamicShadow = true;
	// A/B visual candidate only: a CC-BY review mesh, sanitized before import
	// (license plate and steering emblem removed). FObjectFinder gives the cooker
	// an explicit CDO reference while BodyMesh remains the Chaos collider.
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CarConceptReview(
		TEXT("/Game/ThirdPartyReview/SM_CarConceptReview.SM_CarConceptReview"));
	if (UStaticMesh* ReviewCoupe = CarConceptReview.Succeeded() ? CarConceptReview.Object : nullptr)
	{
		AuthoredBodyMesh->SetStaticMesh(ReviewCoupe);
		AuthoredBodyMesh->SetRelativeLocation(FVector::ZeroVector);
		// Imported review mesh is Y-forward; align it to the X-forward Chaos chassis.
		// Its local bounds (254x436x115 cm) are calibrated to the prior authored
		// body (538x490x112 cm) without changing the physics/collision envelope.
		AuthoredBodyMesh->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
		AuthoredBodyMesh->SetRelativeScale3D(FVector(1.93f, 1.24f, 0.97f));
		// Preserve imported paint/glass/tire/light material slots for the A/B. The
		// procedural BasicShape MIDs are intentionally not applied to this asset.
		UE_LOG(LogTemp, Log, TEXT("NeonDistrict: SM_CarConceptReview loaded for %s"), *GetName());
	}
	else
	{
		AuthoredBodyMesh->SetVisibility(false, false);
		UE_LOG(LogTemp, Error, TEXT("NeonDistrict: SM_CarConceptReview FAILED to load for %s"), *GetName());
	}

	BodyVisualMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("BodyVisualMesh"));
	BodyVisualMesh->SetupAttachment(VehicleRoot);
	BodyVisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BodyVisualMesh->SetCastShadow(true);
	BodyVisualMesh->bCastDynamicShadow = true;
	BuildOriginalBodyVisual();

	CabinMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CabinMesh"));
	CabinMesh->SetupAttachment(VehicleRoot);
	CabinMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CabinMesh->SetRelativeLocation(FVector(-30.0f, 0.0f, 70.0f));
	CabinMesh->SetRelativeScale3D(FVector(1.12f, 0.76f, 0.42f));

	WindshieldMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WindshieldMesh"));
	WindshieldMesh->SetupAttachment(VehicleRoot);
	WindshieldMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WindshieldMesh->SetRelativeLocation(FVector(55.0f, 0.0f, 105.0f));
	WindshieldMesh->SetRelativeRotation(FRotator(0.0f, 0.0f, -27.0f));
	WindshieldMesh->SetRelativeScale3D(FVector(0.96f, 0.90f, 0.075f));

	SpoilerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SpoilerMesh"));
	SpoilerMesh->SetupAttachment(VehicleRoot);
	SpoilerMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SpoilerMesh->SetRelativeLocation(FVector(-176.0f, 0.0f, 78.0f));
	SpoilerMesh->SetRelativeScale3D(FVector(0.55f, 1.25f, 0.08f));

	FrontLightMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FrontLightMesh"));
	FrontLightMesh->SetupAttachment(VehicleRoot);
	FrontLightMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FrontLightMesh->SetRelativeLocation(FVector(162.0f, 0.0f, 18.0f));
	FrontLightMesh->SetRelativeScale3D(FVector(0.08f, 1.08f, 0.10f));

	RearLightMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RearLightMesh"));
	RearLightMesh->SetupAttachment(VehicleRoot);
	RearLightMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RearLightMesh->SetRelativeLocation(FVector(-164.0f, 0.0f, 18.0f));
	RearLightMesh->SetRelativeScale3D(FVector(0.08f, 1.08f, 0.10f));

	// Road-car rear mass: the deck and vertical bumper establish a useful
	// luggage/technical volume behind the canopy instead of a sci-fi tail.
	TrunkDeckMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TrunkDeckMesh"));
	TrunkDeckMesh->SetupAttachment(VehicleRoot);
	TrunkDeckMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TrunkDeckMesh->SetRelativeLocation(FVector(-142.0f, 0.0f, 62.0f));
	TrunkDeckMesh->SetRelativeScale3D(FVector(0.72f, 0.84f, 0.18f));
	TrunkDeckMesh->SetCastShadow(true);
	TrunkDeckMesh->bCastDynamicShadow = true;

	RearBumperMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RearBumperMesh"));
	RearBumperMesh->SetupAttachment(VehicleRoot);
	RearBumperMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RearBumperMesh->SetRelativeLocation(FVector(-192.0f, 0.0f, 15.0f));
	RearBumperMesh->SetRelativeScale3D(FVector(0.14f, 0.92f, 0.34f));
	RearBumperMesh->SetCastShadow(true);
	RearBumperMesh->bCastDynamicShadow = true;

	// Separate visual panels turn the physics cube into a low, planted sports
	// coupe while keeping BodyMesh untouched as Chaos' collision chassis.
	auto AddBodyPanel = [&](const TCHAR* Name, const FVector& Location, const FVector& Scale)
	{
		UStaticMeshComponent* Panel = CreateDefaultSubobject<UStaticMeshComponent>(Name);
		Panel->SetupAttachment(VehicleRoot);
		Panel->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Panel->SetRelativeLocation(Location);
		Panel->SetRelativeScale3D(Scale);
		Panel->SetCastShadow(true);
		Panel->bCastDynamicShadow = true;
		return Panel;
	};
	HoodMesh = AddBodyPanel(TEXT("HoodMesh"), FVector(92.0f, 0.0f, 49.0f), FVector(1.15f, 1.15f, 0.10f));
	NoseMesh = AddBodyPanel(TEXT("NoseMesh"), FVector(182.0f, 0.0f, 34.0f), FVector(0.50f, 1.28f, 0.42f));
	NoseMesh->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
	IntakeLeftMesh = AddBodyPanel(TEXT("IntakeLeftMesh"), FVector(-18.0f, 103.0f, 28.0f), FVector(0.78f, 0.12f, 0.28f));
	IntakeRightMesh = AddBodyPanel(TEXT("IntakeRightMesh"), FVector(-18.0f, -103.0f, 28.0f), FVector(0.78f, 0.12f, 0.28f));
	FrontSplitterMesh = AddBodyPanel(TEXT("FrontSplitterMesh"), FVector(173.0f, 0.0f, -8.0f), FVector(0.18f, 1.35f, 0.05f));
	SideSkirtMesh = AddBodyPanel(TEXT("SideSkirtMesh"), FVector(-5.0f, 0.0f, -8.0f), FVector(2.50f, 1.50f, 0.05f));
	RearDiffuserMesh = AddBodyPanel(TEXT("RearDiffuserMesh"), FVector(-171.0f, 0.0f, -2.0f), FVector(0.20f, 1.22f, 0.08f));
	MirrorLeftMesh = AddBodyPanel(TEXT("MirrorLeftMesh"), FVector(18.0f, 108.0f, 65.0f), FVector(0.18f, 0.22f, 0.10f));
	MirrorRightMesh = AddBodyPanel(TEXT("MirrorRightMesh"), FVector(18.0f, -108.0f, 65.0f), FVector(0.18f, 0.22f, 0.10f));

	auto MakeWheel = [&](const TCHAR* Name, FVector Loc) -> UStaticMeshComponent*
	{
		UStaticMeshComponent* Wheel = CreateDefaultSubobject<UStaticMeshComponent>(Name);
		Wheel->SetupAttachment(VehicleRoot);
		Wheel->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Wheel->SetRelativeLocation(Loc);
		Wheel->SetRelativeRotation(FRotator(0.0f, 0.0f, 90.0f));
		Wheel->SetRelativeScale3D(FVector(0.52f, 0.52f, 0.32f));
		return Wheel;
	};
	WheelFL = MakeWheel(TEXT("WheelFL"), FVector(116.0f, 116.0f, -20.0f));
	WheelFR = MakeWheel(TEXT("WheelFR"), FVector(116.0f, -116.0f, -20.0f));
	WheelRL = MakeWheel(TEXT("WheelRL"), FVector(-126.0f, 116.0f, -20.0f));
	WheelRR = MakeWheel(TEXT("WheelRR"), FVector(-126.0f, -116.0f, -20.0f));

	if (UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, CubeMeshPath))
	{
		CabinMesh->SetStaticMesh(Cube);
		WindshieldMesh->SetStaticMesh(Cube);
		SpoilerMesh->SetStaticMesh(Cube);
		FrontLightMesh->SetStaticMesh(Cube);
		RearLightMesh->SetStaticMesh(Cube);
		TrunkDeckMesh->SetStaticMesh(Cube);
		RearBumperMesh->SetStaticMesh(Cube);
		HoodMesh->SetStaticMesh(Cube);
		IntakeLeftMesh->SetStaticMesh(Cube);
		IntakeRightMesh->SetStaticMesh(Cube);
		FrontSplitterMesh->SetStaticMesh(Cube);
		SideSkirtMesh->SetStaticMesh(Cube);
		RearDiffuserMesh->SetStaticMesh(Cube);
		MirrorLeftMesh->SetStaticMesh(Cube);
		MirrorRightMesh->SetStaticMesh(Cube);
	}
	if (UStaticMesh* Cone = LoadObject<UStaticMesh>(nullptr, ConeMeshPath))
	{
		NoseMesh->SetStaticMesh(Cone);
	}
	if (UStaticMesh* Cylinder = LoadObject<UStaticMesh>(nullptr, CylinderMeshPath))
	{
		WheelFL->SetStaticMesh(Cylinder);
		WheelFR->SetStaticMesh(Cylinder);
		WheelRL->SetStaticMesh(Cylinder);
		WheelRR->SetStaticMesh(Cylinder);
	}
	TintVehiclePart(CabinMesh, this, FLinearColor(0.02f, 0.08f, 0.16f), 0.0f);
	TintVehiclePart(WindshieldMesh, this, FLinearColor(0.04f, 0.40f, 0.70f), 1.0f);
	CabinMesh->SetVisibility(false, false);
	WindshieldMesh->SetVisibility(false, false);
	TintVehiclePart(SpoilerMesh, this, FLinearColor(0.01f, 0.01f, 0.02f), 0.0f);
	SpoilerMesh->SetVisibility(false, false);
	TintVehiclePart(FrontLightMesh, this, FLinearColor(1.0f, 0.92f, 0.55f), 5.0f);
	TintVehiclePart(RearLightMesh, this, FLinearColor(1.0f, 0.04f, 0.08f), 4.0f);
	TintVehiclePart(TrunkDeckMesh, this, FLinearColor(0.018f, 0.20f, 0.70f), 0.08f);
	TintVehiclePart(RearBumperMesh, this, FLinearColor(0.012f, 0.07f, 0.16f), 0.0f);
	TintVehiclePart(HoodMesh, this, FLinearColor(0.03f, 0.34f, 1.0f), 0.15f);
	TintVehiclePart(NoseMesh, this, FLinearColor(0.03f, 0.34f, 1.0f), 0.10f);
	TintVehiclePart(IntakeLeftMesh, this, FLinearColor(0.004f, 0.008f, 0.016f), 0.0f);
	TintVehiclePart(IntakeRightMesh, this, FLinearColor(0.004f, 0.008f, 0.016f), 0.0f);
	TintVehiclePart(FrontSplitterMesh, this, FLinearColor(0.006f, 0.008f, 0.014f), 0.0f);
	TintVehiclePart(SideSkirtMesh, this, FLinearColor(0.006f, 0.008f, 0.014f), 0.0f);
	TintVehiclePart(RearDiffuserMesh, this, FLinearColor(0.006f, 0.008f, 0.014f), 0.0f);
	TintVehiclePart(MirrorLeftMesh, this, FLinearColor(0.03f, 0.34f, 1.0f), 0.0f);
	TintVehiclePart(MirrorRightMesh, this, FLinearColor(0.03f, 0.34f, 1.0f), 0.0f);
	TintVehiclePart(WheelFL, this, FLinearColor(0.005f, 0.005f, 0.008f), 0.0f);
	TintVehiclePart(WheelFR, this, FLinearColor(0.005f, 0.005f, 0.008f), 0.0f);
	TintVehiclePart(WheelRL, this, FLinearColor(0.005f, 0.005f, 0.008f), 0.0f);
	TintVehiclePart(WheelRR, this, FLinearColor(0.005f, 0.005f, 0.008f), 0.0f);

	// The first primitive-kit pass has been superseded by the procedural shell.
	// Keep these objects only for compatibility with existing code; do not let
	// their oversized boxes contaminate the final silhouette.
	for (UStaticMeshComponent* LegacyPanel : { HoodMesh, NoseMesh, IntakeLeftMesh, IntakeRightMesh,
		FrontSplitterMesh, SideSkirtMesh, RearDiffuserMesh, MirrorLeftMesh, MirrorRightMesh })
	{
		if (LegacyPanel)
		{
			LegacyPanel->SetVisibility(false, false);
		}
	}

	if (AuthoredBodyMesh && AuthoredBodyMesh->GetStaticMesh())
	{
		BodyVisualMesh->SetVisibility(false, false);
		for (UStaticMeshComponent* ProceduralPiece : { CabinMesh, WindshieldMesh, SpoilerMesh, FrontLightMesh,
			RearLightMesh, TrunkDeckMesh, RearBumperMesh, HoodMesh, NoseMesh, IntakeLeftMesh, IntakeRightMesh,
			FrontSplitterMesh, SideSkirtMesh, RearDiffuserMesh, MirrorLeftMesh, MirrorRightMesh,
			WheelFL, WheelFR, WheelRL, WheelRR })
		{
			if (ProceduralPiece)
			{
				ProceduralPiece->SetVisibility(false, false);
			}
		}
	}

	VehicleMovement = CreateDefaultSubobject<UChaosWheeledVehicleMovementComponent>(TEXT("VehicleMovement"));
	VehicleMovement->SetUpdatedComponent(BodyMesh);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(VehicleRoot);
	// A vehicle camera must not retract into its own wide body. Disabling the
	// probe yields a stable rear-three-quarter supercar silhouette in gameplay
	// and in the packaged screenshot gate.
	SpringArm->TargetArmLength = 680.0f;
	SpringArm->bDoCollisionTest = false;
	SpringArm->bUsePawnControlRotation = false;
	SpringArm->SetRelativeRotation(FRotator(-12.0f, 0.0f, 0.0f));

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
}

void ANDVehicle::BuildOriginalBodyVisual()
{
	if (!BodyVisualMesh)
	{
		return;
	}

	// Original multi-shoulder loft. Six points per station create separate
	// lower body, wheel-arch shoulder and roof planes instead of a flat slab.
// It is a generic cab-forward coupe, not a reproduction of a real car.
	struct FBodyStation
	{
		float X;
		float HalfWidth;
		float Bottom;
		float Top;
	};
	const TArray<FBodyStation> Stations = {
		{ -190.0f, 44.0f, -22.0f, 28.0f },
		{ -126.0f, 82.0f, -24.0f, 50.0f },
		{  -18.0f, 98.0f, -26.0f, 72.0f },
		{   94.0f, 86.0f, -23.0f, 62.0f },
		{  194.0f, 50.0f, -18.0f, 30.0f }
	};

	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	TArray<FLinearColor> Colors;
	TArray<FProcMeshTangent> Tangents;
	Vertices.Reserve(Stations.Num() * 6);

	for (int32 Index = 0; Index < Stations.Num(); ++Index)
	{
		const FBodyStation& S = Stations[Index];
		const float U = static_cast<float>(Index) / static_cast<float>(Stations.Num() - 1);
		const FVector Ring[6] = {
			FVector(S.X, -S.HalfWidth, S.Bottom),
			FVector(S.X, -S.HalfWidth, FMath::Lerp(S.Bottom, S.Top, 0.58f)),
			FVector(S.X, -S.HalfWidth * 0.78f, S.Top),
			FVector(S.X, S.HalfWidth * 0.78f, S.Top),
			FVector(S.X, S.HalfWidth, FMath::Lerp(S.Bottom, S.Top, 0.58f)),
			FVector(S.X, S.HalfWidth, S.Bottom)
		};
		for (int32 Corner = 0; Corner < 6; ++Corner)
		{
			Vertices.Add(Ring[Corner]);
			const FVector Normal = (Corner == 0 || Corner == 5) ? FVector::DownVector
				: (Corner == 1 ? FVector(0.0f, -1.0f, 0.25f).GetSafeNormal()
				: (Corner == 4 ? FVector(0.0f, 1.0f, 0.25f).GetSafeNormal()
				: (Corner == 2 ? FVector(0.0f, -0.45f, 0.9f).GetSafeNormal()
				: (Corner == 3 ? FVector(0.0f, 0.45f, 0.9f).GetSafeNormal() : FVector::UpVector))));
			Normals.Add(Normal);
			UVs.Add(FVector2D(U, static_cast<float>(Corner) / 5.0f));
			Colors.Add(FLinearColor(0.025f, 0.30f, 0.96f));
			Tangents.Add(FProcMeshTangent(FVector::ForwardVector, false));
		}
	}

	auto AddQuad = [&Triangles](int32 A, int32 B, int32 C, int32 D)
	{
		Triangles.Append({ A, B, C, A, C, D });
	};
	for (int32 Ring = 0; Ring < Stations.Num() - 1; ++Ring)
	{
		const int32 A = Ring * 6;
		const int32 B = A + 6;
		for (int32 Side = 0; Side < 6; ++Side)
		{
			const int32 Next = (Side + 1) % 6;
			AddQuad(A + Side, B + Side, B + Next, A + Next);
		}
	}
	AddQuad(0, 1, 2, 3);
	AddQuad(0, 3, 4, 5);
	const int32 Last = (Stations.Num() - 1) * 6;
	AddQuad(Last + 3, Last + 2, Last + 1, Last);
	AddQuad(Last + 5, Last + 4, Last + 3, Last);

	BodyVisualMesh->CreateMeshSection_LinearColor(0, Vertices, Triangles, Normals, UVs,
		Colors, Tangents, false, false);
	if (UMaterialInterface* Base = LoadObject<UMaterialInterface>(nullptr, BasicShapeMatPath))
	{
		UMaterialInstanceDynamic* Paint = UMaterialInstanceDynamic::Create(Base, this);
		Paint->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.018f, 0.24f, 0.90f));
		Paint->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.018f, 0.24f, 0.90f));
		Paint->SetScalarParameterValue(TEXT("Roughness"), 0.22f);
		Paint->SetScalarParameterValue(TEXT("Metallic"), 0.72f);
		BodyVisualMesh->SetMaterial(0, Paint);
	}

	// Dark glass canopy in its own section: visible from a street-level camera
	// and deliberately taller than the shoulder line so the car never reads as
	// a flat skateboard.
	const TArray<FBodyStation> CanopyStations = {
		{ -108.0f, 42.0f, 66.0f, 82.0f },
		{  -52.0f, 61.0f, 68.0f, 126.0f },
		{   32.0f, 58.0f, 66.0f, 132.0f },
		{   92.0f, 38.0f, 60.0f, 84.0f }
	};
	TArray<FVector> CanopyVertices;
	TArray<int32> CanopyTriangles;
	TArray<FVector> CanopyNormals;
	TArray<FVector2D> CanopyUVs;
	TArray<FLinearColor> CanopyColors;
	TArray<FProcMeshTangent> CanopyTangents;
	for (int32 Index = 0; Index < CanopyStations.Num(); ++Index)
	{
		const FBodyStation& S = CanopyStations[Index];
		const float U = static_cast<float>(Index) / static_cast<float>(CanopyStations.Num() - 1);
		const FVector Ring[4] = {
			FVector(S.X, -S.HalfWidth, S.Bottom), FVector(S.X, S.HalfWidth, S.Bottom),
			FVector(S.X, S.HalfWidth * 0.78f, S.Top), FVector(S.X, -S.HalfWidth * 0.78f, S.Top)
		};
		for (int32 Corner = 0; Corner < 4; ++Corner)
		{
			CanopyVertices.Add(Ring[Corner]);
			CanopyNormals.Add(Corner >= 2 ? FVector::UpVector : FVector(0.0f, 0.0f, -1.0f));
			CanopyUVs.Add(FVector2D(U, static_cast<float>(Corner & 1)));
			CanopyColors.Add(FLinearColor(0.015f, 0.08f, 0.16f));
			CanopyTangents.Add(FProcMeshTangent(FVector::ForwardVector, false));
		}
	}
	auto AddCanopyQuad = [&CanopyTriangles](int32 A, int32 B, int32 C, int32 D)
	{
		CanopyTriangles.Append({ A, B, C, A, C, D });
	};
	for (int32 Ring = 0; Ring < CanopyStations.Num() - 1; ++Ring)
	{
		const int32 A = Ring * 4;
		const int32 B = A + 4;
		AddCanopyQuad(A, B, B + 1, A + 1);
		AddCanopyQuad(A + 1, B + 1, B + 2, A + 2);
		AddCanopyQuad(A + 2, B + 2, B + 3, A + 3);
		AddCanopyQuad(A + 3, B + 3, B, A);
	}
	AddCanopyQuad(0, 1, 2, 3);
	const int32 CanopyLast = (CanopyStations.Num() - 1) * 4;
	AddCanopyQuad(CanopyLast + 3, CanopyLast + 2, CanopyLast + 1, CanopyLast);
	BodyVisualMesh->CreateMeshSection_LinearColor(1, CanopyVertices, CanopyTriangles, CanopyNormals,
		CanopyUVs, CanopyColors, CanopyTangents, false, false);
	if (UMaterialInterface* Base = LoadObject<UMaterialInterface>(nullptr, BasicShapeMatPath))
	{
		UMaterialInstanceDynamic* Glass = UMaterialInstanceDynamic::Create(Base, this);
		Glass->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.01f, 0.045f, 0.10f));
		Glass->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.01f, 0.045f, 0.10f));
		Glass->SetScalarParameterValue(TEXT("Roughness"), 0.08f);
		Glass->SetScalarParameterValue(TEXT("Metallic"), 0.15f);
		BodyVisualMesh->SetMaterial(1, Glass);
	}
}

void ANDVehicle::BeginPlay()
{
	Super::BeginPlay();
	EnsureWheels();
	UE_LOG(LogTemp, Log, TEXT("NeonDistrict: vehicle runtime %s loc=(%.0f, %.0f, %.0f) authored=%s mesh=%s hidden=%s"),
		*GetName(), GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z,
		AuthoredBodyMesh ? TEXT("yes") : TEXT("no"),
		(AuthoredBodyMesh && AuthoredBodyMesh->GetStaticMesh()) ? *AuthoredBodyMesh->GetStaticMesh()->GetName() : TEXT("none"),
		(AuthoredBodyMesh && AuthoredBodyMesh->bHiddenInGame) ? TEXT("true") : TEXT("false"));
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

	// Remember who was driving so ExitVehicle can put the player back on foot.
	PreviousPawn = PC->GetPawn();

	if (ANDPlayerController* NDPC = Cast<ANDPlayerController>(PC))
	{
		NDPC->SetDrivingState(this, true);
	}

	PC->Possess(this);
	PC->SetControlRotation(GetActorRotation());
	PC->SetViewTarget(this);

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
		NDPC->SetDrivingState(this, false); // clears driving state
	}

	// Re-possess the character that was driving. The player controller must end
	// the interaction back on the playable pawn or movement/input stay dead.
	if (PC && PreviousPawn)
	{
		PC->Possess(PreviousPawn);
		PC->SetControlRotation(PreviousPawn->GetActorRotation());
		PC->SetViewTarget(PreviousPawn);
		PreviousPawn = nullptr;
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
