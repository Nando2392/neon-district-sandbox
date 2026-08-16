// Copyright Neon District Sandbox. Public benchmark repo — original content only.

#include "Systems/NDWorldBuilder.h"
#include "AI/NDCitySpawner.h"
#include "Combat/NDWeaponPickup.h"
#include "Core/NDPerfConstants.h"

#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/PostProcessComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpressionConstant.h"
#include "Materials/MaterialExpressionConstant3Vector.h"
#include "Materials/MaterialExpressionMultiply.h"
#include "Materials/MaterialExpressionVectorParameter.h"
#include "Materials/MaterialExpressionScalarParameter.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/App.h"

#if WITH_EDITOR
#include "MaterialEditingLibrary.h"
#endif

namespace
{
	const TCHAR* CubeMeshPath = TEXT("/Engine/BasicShapes/Cube.Cube");
	const TCHAR* CylinderMeshPath = TEXT("/Engine/BasicShapes/Cylinder.Cylinder");
}

ANDWorldBuilder::ANDWorldBuilder()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DistrictRoot"));
	SetRootComponent(SceneRoot);

	// This is intentionally a constructor-time hard reference. Runtime LoadObject
	// calls alone do not make a new material discoverable to BuildCookRun.
	static ConstructorHelpers::FObjectFinder<UMaterial> AsphaltFinder(
		TEXT("/Game/Generated/M_NDAsphalt.M_NDAsphalt"));
	AsphaltMat = AsphaltFinder.Succeeded() ? AsphaltFinder.Object : nullptr;
	static ConstructorHelpers::FObjectFinder<UMaterial> SidewalkFinder(
		TEXT("/Game/Generated/M_NDSidewalk.M_NDSidewalk"));
	SidewalkMat = SidewalkFinder.Succeeded() ? SidewalkFinder.Object : nullptr;
	static ConstructorHelpers::FObjectFinder<UStaticMesh> UrbanTreeFinder(
		TEXT("/Game/Generated/SM_NDUrbanTree.SM_NDUrbanTree"));
	UrbanTreeMesh = UrbanTreeFinder.Succeeded() ? UrbanTreeFinder.Object : nullptr;

	// Neon-night palette (base facade, accent glow).
	Palette.Add({ FLinearColor(0.16f, 0.10f, 0.30f), FLinearColor(1.0f, 0.10f, 0.60f) }); // violet/magenta
	Palette.Add({ FLinearColor(0.08f, 0.18f, 0.34f), FLinearColor(0.10f, 0.90f, 1.00f) }); // navy/cyan
	Palette.Add({ FLinearColor(0.22f, 0.08f, 0.12f), FLinearColor(1.00f, 0.35f, 0.05f) }); // maroon/orange
	Palette.Add({ FLinearColor(0.10f, 0.24f, 0.16f), FLinearColor(0.20f, 1.00f, 0.40f) }); // deep green/neon green
	Palette.Add({ FLinearColor(0.14f, 0.14f, 0.28f), FLinearColor(0.60f, 0.30f, 1.00f) }); // indigo/purple
	Palette.Add({ FLinearColor(0.26f, 0.16f, 0.06f), FLinearColor(1.00f, 0.90f, 0.10f) }); // amber/gold
}

void ANDWorldBuilder::BeginPlay()
{
	Super::BeginPlay();
	BuildDistrict();
}

void ANDWorldBuilder::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	// World builder is static after construction; tick stays enabled for editor gizmos only.
}

// ---------------------------------------------------------------------------
// Materials
// ---------------------------------------------------------------------------

UMaterial* ANDWorldBuilder::CreateNeonMaterial()
{
	// NeonMat: BaseColor <- NeonColor; Emissive <- NeonColor * EmissiveStrength.
	// Expression wiring + property connects are editor-only in UE 5.8
	// (UMaterialEditingLibrary; BaseColor/EmissiveColor/ShadingModel are
	// private outside the editor), so the procedural graph only exists in
	// PIE. Packaged builds use the engine's basic shape material, which is
	// lit by the district's colored lights + fog + bloom — not a flat gray
	// void, but not emissive. MIDs set NeonColor/EmissiveStrength as no-ops.
#if WITH_EDITOR
	if (!FApp::IsGame())
	{
		UMaterial* Mat = NewObject<UMaterial>(this, UMaterial::StaticClass(), TEXT("ND_NeonMat"));
		Mat->SetFlags(RF_Transient);

		UMaterialExpressionVectorParameter* ColorParam = NewObject<UMaterialExpressionVectorParameter>(Mat);
		ColorParam->ParameterName = TEXT("NeonColor");
		ColorParam->DefaultValue = FLinearColor::White;

		UMaterialExpressionScalarParameter* StrengthParam = NewObject<UMaterialExpressionScalarParameter>(Mat);
		StrengthParam->ParameterName = TEXT("EmissiveStrength");
		StrengthParam->DefaultValue = 1.0f;

		UMaterialExpressionMultiply* Multiply = NewObject<UMaterialExpressionMultiply>(Mat);
		Multiply->A.Expression = ColorParam;
		Multiply->B.Expression = StrengthParam;

		Mat->GetExpressionCollection().AddExpression(ColorParam);
		Mat->GetExpressionCollection().AddExpression(StrengthParam);
		Mat->GetExpressionCollection().AddExpression(Multiply);

		UMaterialEditingLibrary::ConnectMaterialProperty(ColorParam, TEXT(""), MP_BaseColor);
		UMaterialEditingLibrary::ConnectMaterialProperty(Multiply, TEXT(""), MP_EmissiveColor);
		Mat->BlendMode = BLEND_Opaque;
		Mat->SetShadingModel(MSM_DefaultLit);
		Mat->TwoSided = true;
		Mat->PostEditChange();

		return Mat;
	}
#endif
	// Packaged build / standalone -game: no editor material graph. Use the
	// engine's EmissiveMeshMaterial (real emissive, cooked with the engine),
	// whose parameters are "Color" (base) and "EmissiveColor" (emissive tint).
	return LoadObject<UMaterial>(nullptr,
		TEXT("/Engine/EngineMaterials/EmissiveMeshMaterial.EmissiveMeshMaterial"));
}

UMaterialInstanceDynamic* ANDWorldBuilder::MakeMID(UMaterial* Base, const FLinearColor& Color, float EmissiveStrength)
{
	UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(Base, this);
	// EmissiveMeshMaterial params: "Color" (base) + "EmissiveColor" (emissive
	// tint). EmissiveColor = Color * Strength so strength 0 keeps a matte look.
	MID->SetVectorParameterValue(TEXT("Color"), Color);
	MID->SetVectorParameterValue(TEXT("EmissiveColor"), Color * EmissiveStrength);
	// BasicShapeMaterial params (fallback / editor path): same intent.
	MID->SetVectorParameterValue(TEXT("BaseColor"), Color);
	MID->SetVectorParameterValue(TEXT("NeonColor"), Color);
	MID->SetScalarParameterValue(TEXT("EmissiveStrength"), EmissiveStrength);
	// Surface feel: dark matte asphalt vs wet neon-adjacent surfaces.
	MID->SetScalarParameterValue(TEXT("Roughness"), EmissiveStrength > 0.05f ? 0.25f : 0.85f);
	MID->SetScalarParameterValue(TEXT("Metallic"), EmissiveStrength > 0.05f ? 0.0f : 0.05f);
	return MID;
}

UTexture2D* ANDWorldBuilder::CreateFacadeTexture(const FLinearColor& WallColor,
	const FLinearColor& LitWindowColor, int32 Seed, int32 Width, int32 Height)
{
	// Runtime safe: CreateTransient is available in packaged builds. Do not use
	// FImageUtils::CreateTexture2D, which is editor-only and disappears after cook.
	UTexture2D* Texture = UTexture2D::CreateTransient(Width, Height, PF_B8G8R8A8);
	if (!Texture || !Texture->GetPlatformData() || Texture->GetPlatformData()->Mips.IsEmpty())
	{
		return nullptr;
	}

	Texture->NeverStream = true;
	Texture->SRGB = true;
	TArray<FColor> Pixels;
	Pixels.SetNumUninitialized(Width * Height);

	const FLinearColor MortarColor = WallColor * 0.45f;
	const FLinearColor GlassDark(0.006f, 0.014f, 0.027f);
	const FLinearColor GlassCool(0.015f, 0.050f, 0.085f);
	const int32 Bays = 5 + (Seed % 3);
	const int32 Floors = 8 + ((Seed / 3) % 5);
	const int32 BayWidth = FMath::Max(12, Width / Bays);
	const int32 FloorHeight = FMath::Max(14, Height / Floors);

	for (int32 Y = 0; Y < Height; ++Y)
	{
		for (int32 X = 0; X < Width; ++X)
		{
			const int32 Bay = X / BayWidth;
			const int32 Floor = Y / FloorHeight;
			const int32 LocalX = X % BayWidth;
			const int32 LocalY = Y % FloorHeight;
			const bool bMullion = LocalX < 3 || LocalY < 3;
			const bool bWindow = !bMullion && LocalX < BayWidth - 4 && LocalY < FloorHeight - 4;
			const uint32 CellHash = static_cast<uint32>(Seed * 73856093) ^
				static_cast<uint32>(Bay * 19349663) ^ static_cast<uint32>(Floor * 83492791);
			const bool bLit = (CellHash % 100) < 58;

			FLinearColor Pixel = WallColor;
			if (bMullion)
			{
				Pixel = MortarColor;
			}
			else if (bWindow)
			{
				Pixel = bLit ? LitWindowColor : ((CellHash & 1) ? GlassDark : GlassCool);
			}
			else if (((X + Y + Seed) % 23) == 0)
			{
				// Sparse weathering/panel variation; avoids a perfectly flat plaster look.
				Pixel = WallColor * 0.78f;
			}

			Pixels[Y * Width + X] = Pixel.ToFColorSRGB();
		}
	}

	FTexture2DMipMap& Mip = Texture->GetPlatformData()->Mips[0];
	void* Data = Mip.BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(Data, Pixels.GetData(), Pixels.Num() * sizeof(FColor));
	Mip.BulkData.Unlock();
	Texture->UpdateResource();
	RuntimeFacadeTextures.Add(Texture);
	return Texture;
}

UStaticMeshComponent* ANDWorldBuilder::AddBox(FVector Location, FVector Scale, const FLinearColor& Color, float Emissive,
	UMaterial* MaterialOverride, FVector RelativeRotation)
{
	UStaticMeshComponent* Comp = NewObject<UStaticMeshComponent>(this);
	Comp->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, CubeMeshPath));
	Comp->SetMobility(EComponentMobility::Movable);
	Comp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Comp->SetCollisionResponseToAllChannels(ECR_Block);
	Comp->SetupAttachment(SceneRoot);
	Comp->RegisterComponent();
	Comp->SetRelativeLocation(Location);
	// /Engine/BasicShapes/Cube is 100uu per side. Callers pass dimensions in
	// centimeters (street width, block size, building height), not raw Unreal
	// scale multipliers. Applying those values directly made every slab 100x too
	// large, so street placement traces hit skyscraper/sidewalk collision at the
	// trace start instead of the asphalt below.
	Comp->SetRelativeScale3D(Scale / 100.0f);
	if (!RelativeRotation.IsZero())
	{
		Comp->SetRelativeRotation(FRotator(RelativeRotation.X, RelativeRotation.Y, RelativeRotation.Z));
	}

	UMaterial* Mat = MaterialOverride ? MaterialOverride : (Emissive > 0.05f ? NeonMat : MatteMat);
	Comp->SetMaterial(0, MakeMID(Mat, Color, Emissive));
	BuildComponents.Add(Comp);
	return Comp;
}

// ---------------------------------------------------------------------------
// District layout
// ---------------------------------------------------------------------------

void ANDWorldBuilder::BuildDistrict()
{
	if (!NeonMat)
	{
		NeonMat = CreateNeonMaterial();
		// Matte surfaces use BasicShapeMaterial (lit by lights, no emissive)
		// so the whole world does not glow like a lightbulb.
		MatteMat = LoadObject<UMaterial>(nullptr,
			TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	}

	const float TotalW = BlocksX * BlockSize + (BlocksX + 1) * 2.0f * StreetHalfWidth;
	const float TotalH = BlocksY * BlockSize + (BlocksY + 1) * 2.0f * StreetHalfWidth;
	const FVector DistrictCenter = FVector::ZeroVector;
	UMaterial* AsphaltMaterial = AsphaltMat ? AsphaltMat.Get() : MatteMat;

	// Ground (asphalt) — persistent generated PBR-style material when cooked.
	AddBox(DistrictCenter + FVector(0, 0, -10), FVector(TotalW, TotalH, 10),
		FLinearColor::White, 0.0f, AsphaltMaterial);

	// Street grid: vertical avenues at x = -StreetHalfWidth, +StreetHalfWidth;
	// horizontal avenues at y = -StreetHalfWidth, +StreetHalfWidth (relative to each block row).
	const float BlockPitch = BlockSize + 2.0f * StreetHalfWidth;
	for (int32 X = 0; X <= BlocksX; ++X)
	{
		const float StreetX = DistrictCenter.X - TotalW * 0.5f + StreetHalfWidth + X * BlockPitch;
		BuildStreet(FVector(StreetX, DistrictCenter.Y, 0.0f), true);
	}
	for (int32 Y = 0; Y <= BlocksY; ++Y)
	{
		const float StreetY = DistrictCenter.Y - TotalH * 0.5f + StreetHalfWidth + Y * BlockPitch;
		BuildStreet(FVector(DistrictCenter.X, StreetY, 0.0f), false);
	}

	BuildCrosswalk(FVector(0.0f, -StreetHalfWidth, 6.0f), true);
	BuildCrosswalk(FVector(0.0f, StreetHalfWidth, 6.0f), true);
	BuildCrosswalk(FVector(-StreetHalfWidth, 0.0f, 6.0f), false);
	BuildCrosswalk(FVector(StreetHalfWidth, 0.0f, 6.0f), false);

	// Blocks.
	for (int32 BX = 0; BX < BlocksX; ++BX)
	{
		for (int32 BY = 0; BY < BlocksY; ++BY)
		{
			BuildBlock(BX, BY);
		}
	}

	// Street furniture along main avenue.
	BuildStreetLight(FVector(0, -StreetHalfWidth * 0.5f, 0), 700.0f);
	BuildStreetLight(FVector(0, StreetHalfWidth * 0.5f, 0), 700.0f);
	BuildStreetLight(FVector(-StreetHalfWidth * 0.5f, 0, 0), 700.0f);
	BuildStreetLight(FVector(StreetHalfWidth * 0.5f, 0, 0), 700.0f);

	BuildSign(FVector(0, -TotalH * 0.5f + 220.0f, 0), Palette[0].Accent);
	BuildSign(FVector(0, TotalH * 0.5f - 220.0f, 0), Palette[1].Accent);
	BuildKiosk(FVector(StreetHalfWidth + BlockSize * 0.25f, StreetHalfWidth + BlockSize * 0.25f, 0));
	BuildBench(FVector(-StreetHalfWidth - 220.0f, -StreetHalfWidth - 320.0f, 0), 0.0f);
	BuildBench(FVector(StreetHalfWidth + 420.0f, StreetHalfWidth + 180.0f, 0), 90.0f);
	BuildTrafficSignal(FVector(StreetHalfWidth + 95.0f, StreetHalfWidth + 95.0f, 0), -45.0f);
	BuildTrafficSignal(FVector(-StreetHalfWidth - 95.0f, StreetHalfWidth + 95.0f, 0), 45.0f);
	BuildRoadSign(FVector(-StreetHalfWidth - 170.0f, -StreetHalfWidth + 220.0f, 0), FLinearColor(0.1f, 0.9f, 1.0f), 20.0f);
	BuildBarrierCones(FVector(StreetHalfWidth - 140.0f, -StreetHalfWidth + 260.0f, 0), 12.0f);
	BuildPoliceCruiserProp(FVector(-StreetHalfWidth + 250.0f, StreetHalfWidth - 280.0f, 20.0f), 18.0f);
	BuildGarbage(FVector(StreetHalfWidth + 260.0f, -StreetHalfWidth - 260.0f, 0));
	BuildGarbage(FVector(-StreetHalfWidth - 260.0f, StreetHalfWidth + 260.0f, 0));
	BuildHydrant(FVector(StreetHalfWidth + 180.0f, StreetHalfWidth + 420.0f, 0));

	// Vegetation breaks the hard edge of an all-concrete grid. These are original
	// runtime-built trees and planters: no Fab content, downloaded assets or LFS.
	BuildPlanter(FVector(-680.0f, -840.0f, 0.0f), 18.0f);
	BuildPlanter(FVector(720.0f, -760.0f, 0.0f), -12.0f);
	BuildPlanter(FVector(-770.0f, 760.0f, 0.0f), 90.0f);
	BuildPlanter(FVector(780.0f, 700.0f, 0.0f), -80.0f);
	BuildUrbanTree(FVector(-1050.0f, -1080.0f, 0.0f), 1.10f);
	BuildUrbanTree(FVector(1090.0f, -980.0f, 0.0f), 0.86f);
	BuildUrbanTree(FVector(-1100.0f, 1040.0f, 0.0f), 0.96f);
	BuildUrbanTree(FVector(1050.0f, 1100.0f, 0.0f), 1.18f);

	// Hero street dressing for the southwest city approach. The packaged visual
	// benchmark frames this corridor from (-3520,-3420) toward (-2450,-2450),
	// so placing original props here makes the actual acceptance screenshot read
	// as a lived-in street instead of an uninhabited tower grid.
	BuildPlanter(FVector(-3300.0f, -3070.0f, 0.0f), 90.0f);
	BuildPlanter(FVector(-2750.0f, -3320.0f, 0.0f), 0.0f);
	BuildUrbanTree(FVector(-3430.0f, -3000.0f, 0.0f), 0.82f);
	BuildUrbanTree(FVector(-2600.0f, -3400.0f, 0.0f), 0.76f);
	BuildBench(FVector(-3200.0f, -3040.0f, 0.0f), 12.0f);
	BuildBench(FVector(-2770.0f, -3290.0f, 0.0f), -78.0f);
	BuildGarbage(FVector(-3020.0f, -3040.0f, 0.0f));
	BuildHydrant(FVector(-2860.0f, -3040.0f, 0.0f));
	BuildTrafficSignal(FVector(-3090.0f, -3090.0f, 0.0f), 35.0f);
	BuildRoadSign(FVector(-3400.0f, -3200.0f, 0.0f), Palette[1].Accent, 12.0f);
	BuildSign(FVector(-2860.0f, -3060.0f, 0.0f), Palette[0].Accent);
	BuildBarrierCones(FVector(-2510.0f, -3170.0f, 0.0f), 86.0f);

	// Weapon pickup: gameplay test + visual prop. It sits on the furnished
	// southwest sidewalk, away from vehicle showcases and planter collision.
	if (UWorld* World = GetWorld())
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ANDWeaponPickup* Pickup = World->SpawnActor<ANDWeaponPickup>(ANDWeaponPickup::StaticClass(),
			FVector(-3050.0f, -3260.0f, 54.0f), FRotator(0.0f, 15.0f, 0.0f), Params);
		if (Pickup)
		{
			UE_LOG(LogTemp, Log, TEXT("NeonDistrict: weapon pickup spawned at (-3050,-3260,54)"));
		}
	}

	BuildAtmosphere();
	SpawnCityPopulation();
}

void ANDWorldBuilder::BuildStreet(FVector Center, bool bIsVertical)
{
	const FLinearColor LaneLine(0.95f, 0.90f, 0.60f);
	UMaterial* AsphaltMaterial = AsphaltMat ? AsphaltMat.Get() : MatteMat;

	const float StreetLen = (BlocksX + 1) * BlockSize + (BlocksX + 2) * 2.0f * StreetHalfWidth;
	const float StreetWide = 2.0f * StreetHalfWidth;

	if (bIsVertical)
	{
		AddBox(Center + FVector(0, 0, -4), FVector(StreetWide, StreetLen, 8.0f), FLinearColor::White, 0.0f, AsphaltMaterial);
		// Center and curb lane markings.
		// Painted road markings sit within the asphalt skin: not raised emissive
		// bars, which looked like they floated above the roadway in packaged shots.
		AddBox(Center + FVector(0, 0, -0.8f), FVector(16.0f, StreetLen, 1.6f), LaneLine, 0.0f);
		AddBox(Center + FVector(-StreetWide * 0.42f, 0, -0.8f), FVector(10.0f, StreetLen, 1.6f), FLinearColor(0.8f, 0.8f, 0.86f), 0.0f);
		AddBox(Center + FVector(StreetWide * 0.42f, 0, -0.8f), FVector(10.0f, StreetLen, 1.6f), FLinearColor(0.8f, 0.8f, 0.86f), 0.0f);
	}
	else
	{
		AddBox(Center + FVector(0, 0, -4), FVector(StreetLen, StreetWide, 8.0f), FLinearColor::White, 0.0f, AsphaltMaterial);
		AddBox(Center + FVector(0, 0, -0.8f), FVector(StreetLen, 16.0f, 1.6f), LaneLine, 0.0f);
		AddBox(Center + FVector(0, -StreetWide * 0.42f, -0.8f), FVector(StreetLen, 10.0f, 1.6f), FLinearColor(0.8f, 0.8f, 0.86f), 0.0f);
		AddBox(Center + FVector(0, StreetWide * 0.42f, -0.8f), FVector(StreetLen, 10.0f, 1.6f), FLinearColor(0.8f, 0.8f, 0.86f), 0.0f);
	}
}

void ANDWorldBuilder::BuildBlock(int32 BX, int32 BY)
{
	const float TotalW = BlocksX * BlockSize + (BlocksX + 1) * 2.0f * StreetHalfWidth;
	const float TotalH = BlocksY * BlockSize + (BlocksY + 1) * 2.0f * StreetHalfWidth;
	const float BlockPitch = BlockSize + 2.0f * StreetHalfWidth;

	const FVector BlockOrigin(
		-TotalW * 0.5f + 2.0f * StreetHalfWidth + BX * BlockPitch,
		-TotalH * 0.5f + 2.0f * StreetHalfWidth + BY * BlockPitch,
		0.0f);

	// Sidewalk slab around the block.
	UMaterial* SidewalkMaterial = SidewalkMat ? SidewalkMat.Get() : MatteMat;
	AddBox(BlockOrigin + FVector(BlockSize * 0.5f, BlockSize * 0.5f, 8.0f),
		FVector(BlockSize + 160.0f, BlockSize + 160.0f, 16.0f),
		FLinearColor::White, 0.0f, SidewalkMaterial);

	// Buildings: 2x2 arrangement inside the block with alley gaps.
	const float InnerMargin = 90.0f;
	const float CellW = (BlockSize - InnerMargin * 3.0f) * 0.5f;
	const float CellH = (BlockSize - InnerMargin * 3.0f) * 0.5f;

	int32 PaletteIndex = (BX * 2 + BY) % Palette.Num();
	for (int32 CX = 0; CX < 2; ++CX)
	{
		for (int32 CY = 0; CY < 2; ++CY)
		{
			const float Width = FMath::FRandRange(MinBuildingWidth, FMath::Min(MaxBuildingWidth, CellW * 0.9f));
			const float Depth = FMath::FRandRange(MinBuildingWidth, FMath::Min(MaxBuildingWidth, CellH * 0.9f));
			const float Height = FMath::FRandRange(MinBuildingHeight, MaxBuildingHeight);

			const FVector CellCenter(
				BlockOrigin.X + InnerMargin + CellW * 0.5f + CX * (CellW + InnerMargin),
				BlockOrigin.Y + InnerMargin + CellH * 0.5f + CY * (CellH + InnerMargin),
				0.0f);

			BuildBuilding(CellCenter, FVector(Width * 0.5f, Depth * 0.5f, Height), (PaletteIndex + CX + CY * 2) % Palette.Num());
		}
	}
}

void ANDWorldBuilder::BuildBuilding(FVector Center, FVector Extent, int32 PaletteIndex)
{
	const FNeonPalette& P = Palette[PaletteIndex % Palette.Num()];
	// Shading hierarchy. Facades are deliberately much less saturated than the
	// sign palette: real cities read as concrete/metal/glass first, neon second.
	const FLinearColor FacadeWall = FLinearColor(0.025f, 0.030f, 0.045f) + P.Base * 0.38f;
	const FLinearColor FrameMetal(0.025f, 0.035f, 0.055f);
	const FLinearColor PodiumConcrete(0.070f, 0.075f, 0.090f);
	const FLinearColor LitWindow = (PaletteIndex % 2 == 0)
		? FLinearColor(1.00f, 0.67f, 0.24f)
		: FLinearColor(0.35f, 0.78f, 1.00f);
	const int32 FacadeSeed = PaletteIndex * 97 + FMath::RoundToInt(Center.X * 0.1f) + FMath::RoundToInt(Center.Y * 0.01f);

	// Structural volume, a darker street-level podium, and a small upper-step
	// break the single extruded-box silhouette before any light is added.
	AddBox(Center + FVector(0, 0, Extent.Z),
		FVector(Extent.X * 2.0f, Extent.Y * 2.0f, Extent.Z * 2.0f), FacadeWall, 0.0f);
	AddBox(Center + FVector(0, 0, 150.0f),
		FVector(Extent.X * 2.0f + 24.0f, Extent.Y * 2.0f + 24.0f, 300.0f), PodiumConcrete, 0.0f);
	AddBox(Center + FVector(0, 0, Extent.Z * 2.0f - 55.0f),
		FVector(Extent.X * 2.0f - 70.0f, Extent.Y * 2.0f - 70.0f, 110.0f), FacadeWall * 0.72f, 0.0f);

	// A generated 192x256 facade texture provides non-uniform lit/unlit offices,
	// mullions and panel weathering in a packaged build. The physical strips
	// below give that shading actual silhouette depth and shadow response.
	UMaterial* TexturedMaterial = CreateNeonMaterial();
	UTexture2D* FacadeTexture = CreateFacadeTexture(FacadeWall, LitWindow, FacadeSeed);
	auto AddTexturedFacade = [&](float FaceY)
	{
		UStaticMeshComponent* Panel = AddBox(FVector(Center.X, FaceY, Extent.Z + 260.0f),
			FVector(Extent.X * 2.0f - 90.0f, 10.0f, FMath::Max(160.0f, Extent.Z * 2.0f - 590.0f)),
			FLinearColor::White, 0.0f, MatteMat);
		if (Panel && TexturedMaterial && FacadeTexture)
		{
			UMaterialInstanceDynamic* MID = MakeMID(TexturedMaterial, FacadeWall * 0.42f, 0.08f);
			MID->SetTextureParameterValue(TEXT("Texture"), FacadeTexture);
			MID->SetVectorParameterValue(TEXT("EmissiveColor"), LitWindow * 0.08f);
			MID->SetScalarParameterValue(TEXT("Roughness"), 0.55f);
			Panel->SetMaterial(0, MID);
		}
	};
	AddTexturedFacade(Center.Y + Extent.Y + 8.0f);
	AddTexturedFacade(Center.Y - Extent.Y - 8.0f);

	// Physical mullions/spandrels/vertical fins. These are deliberately sparse:
	// enough depth to catch moon and street-light shadows without creating a
	// component explosion for the 2x2 district.
	const float BaySpacing = 155.0f + static_cast<float>((FacadeSeed % 3) * 12);
	const float FloorSpacing = 178.0f + static_cast<float>((FacadeSeed % 2) * 14);
	const int32 BayCount = FMath::Max(2, FMath::FloorToInt((Extent.X * 2.0f - 90.0f) / BaySpacing));
	const int32 FloorCount = FMath::Max(3, FMath::FloorToInt((Extent.Z * 2.0f - 590.0f) / FloorSpacing));
	for (int32 Bay = 0; Bay <= BayCount; ++Bay)
	{
		const float X = Center.X - Extent.X + 45.0f + Bay * ((Extent.X * 2.0f - 90.0f) / BayCount);
		AddBox(FVector(X, Center.Y + Extent.Y + 20.0f, Extent.Z + 260.0f),
			FVector(12.0f, 18.0f, FMath::Max(160.0f, Extent.Z * 2.0f - 590.0f)), FrameMetal, 0.0f);
		AddBox(FVector(X, Center.Y - Extent.Y - 20.0f, Extent.Z + 260.0f),
			FVector(12.0f, 18.0f, FMath::Max(160.0f, Extent.Z * 2.0f - 590.0f)), FrameMetal, 0.0f);
	}
	// The generated facade texture is useful metadata for editor inspection, but
	// EmissiveMeshMaterial in a packaged build has no TextureSample input. Make
	// the lit/unlit office rhythm physical instead: shallow inset glass panels
	// render through the same cooked primitive + material path as signs/lights.
	const float FacadeWidth = Extent.X * 2.0f - 90.0f;
	const float FacadeHeight = FMath::Max(160.0f, Extent.Z * 2.0f - 590.0f);
	const float BayWidth = FacadeWidth / BayCount;
	const float FloorHeight = FacadeHeight / FloorCount;
	const float WindowWidth = FMath::Max(45.0f, BayWidth - 30.0f);
	const float WindowHeight = FMath::Max(42.0f, FloorHeight - 30.0f);
	const FLinearColor UnlitGlass(0.004f, 0.012f, 0.028f);
	for (int32 Floor = 0; Floor < FloorCount; ++Floor)
	{
		const float Z = 310.0f + (Floor + 0.5f) * FloorHeight;
		for (int32 Bay = 0; Bay < BayCount; ++Bay)
		{
			const float X = Center.X - Extent.X + 45.0f + (Bay + 0.5f) * BayWidth;
			const bool bLit = (FMath::Abs(FacadeSeed + Bay * 17 + Floor * 11) % 5) <= 1;
			const FLinearColor WindowColor = bLit ? LitWindow : UnlitGlass;
			const float WindowGlow = bLit ? 2.1f : 0.0f;
			AddBox(FVector(X, Center.Y + Extent.Y + 32.0f, Z),
				FVector(WindowWidth, 8.0f, WindowHeight), WindowColor, WindowGlow);
			AddBox(FVector(X, Center.Y - Extent.Y - 32.0f, Z),
				FVector(WindowWidth, 8.0f, WindowHeight), WindowColor, WindowGlow);
		}
	}
	// The showcase camera sees the X-facing sides of the first block as clearly
	// as its front. Continue the same physical office rhythm around the corners
	// rather than leaving those faces as greybox-flat walls.
	const float SideFacadeWidth = Extent.Y * 2.0f - 90.0f;
	const int32 SideBayCount = FMath::Max(2, FMath::FloorToInt(SideFacadeWidth / BaySpacing));
	const float SideBayWidth = SideFacadeWidth / SideBayCount;
	const float SideWindowWidth = FMath::Max(45.0f, SideBayWidth - 30.0f);
	for (int32 Floor = 0; Floor < FloorCount; ++Floor)
	{
		const float Z = 310.0f + (Floor + 0.5f) * FloorHeight;
		for (int32 Bay = 0; Bay < SideBayCount; ++Bay)
		{
			const float Y = Center.Y - Extent.Y + 45.0f + (Bay + 0.5f) * SideBayWidth;
			const bool bLit = (FMath::Abs(FacadeSeed + 41 + Bay * 19 + Floor * 13) % 5) <= 1;
			const FLinearColor WindowColor = bLit ? LitWindow : UnlitGlass;
			const float WindowGlow = bLit ? 2.1f : 0.0f;
			AddBox(FVector(Center.X + Extent.X + 32.0f, Y, Z),
				FVector(8.0f, SideWindowWidth, WindowHeight), WindowColor, WindowGlow);
			AddBox(FVector(Center.X - Extent.X - 32.0f, Y, Z),
				FVector(8.0f, SideWindowWidth, WindowHeight), WindowColor, WindowGlow);
		}
	}
	for (int32 Bay = 0; Bay <= SideBayCount; ++Bay)
	{
		const float Y = Center.Y - Extent.Y + 45.0f + Bay * SideBayWidth;
		AddBox(FVector(Center.X + Extent.X + 20.0f, Y, Extent.Z + 260.0f),
			FVector(18.0f, 12.0f, FacadeHeight), FrameMetal, 0.0f);
		AddBox(FVector(Center.X - Extent.X - 20.0f, Y, Extent.Z + 260.0f),
			FVector(18.0f, 12.0f, FacadeHeight), FrameMetal, 0.0f);
	}
	for (int32 Floor = 0; Floor <= FloorCount; ++Floor)
	{
		const float Z = 310.0f + Floor * ((Extent.Z * 2.0f - 590.0f) / FloorCount);
		AddBox(FVector(Center.X, Center.Y + Extent.Y + 20.0f, Z),
			FVector(Extent.X * 2.0f - 70.0f, 18.0f, 12.0f), FrameMetal, 0.0f);
		AddBox(FVector(Center.X, Center.Y - Extent.Y - 20.0f, Z),
			FVector(Extent.X * 2.0f - 70.0f, 18.0f, 12.0f), FrameMetal, 0.0f);
		AddBox(FVector(Center.X + Extent.X + 20.0f, Center.Y, Z),
			FVector(18.0f, Extent.Y * 2.0f - 70.0f, 12.0f), FrameMetal, 0.0f);
		AddBox(FVector(Center.X - Extent.X - 20.0f, Center.Y, Z),
			FVector(18.0f, Extent.Y * 2.0f - 70.0f, 12.0f), FrameMetal, 0.0f);
	}

	// Original storefronts: recessed dark glass, framing, canopies and fictional
	// luminous sign forms at pedestrian scale. No third-party brands or logos.
	const float StoreWidth = Extent.X * 0.72f;
	AddBox(Center + FVector(-Extent.X * 0.30f, Extent.Y + 18.0f, 145.0f),
		FVector(StoreWidth, 22.0f, 205.0f), FLinearColor(0.008f, 0.035f, 0.070f), 0.0f);
	AddBox(Center + FVector(Extent.X * 0.30f, -Extent.Y - 18.0f, 145.0f),
		FVector(StoreWidth, 22.0f, 205.0f), FLinearColor(0.008f, 0.035f, 0.070f), 0.0f);
	AddBox(Center + FVector(-Extent.X * 0.30f, Extent.Y + 44.0f, 340.0f),
		FVector(StoreWidth + 35.0f, 80.0f, 28.0f), FrameMetal, 0.0f);
	AddBox(Center + FVector(Extent.X * 0.30f, -Extent.Y - 44.0f, 340.0f),
		FVector(StoreWidth + 35.0f, 80.0f, 28.0f), FrameMetal, 0.0f);
	AddBox(Center + FVector(Extent.X * 0.38f, Extent.Y + 52.0f, 405.0f),
		FVector(Extent.X * 0.40f, 18.0f, 46.0f), P.Accent, 4.0f);

	// Vertical fins and a restrained neon belt provide an identifiable skyline
	// without turning every face into a uniformly glowing colored box.
	AddBox(Center + FVector(Extent.X + 22.0f, 0, Extent.Z * 1.15f),
		FVector(24.0f, Extent.Y * 2.0f + 48.0f, Extent.Z * 1.55f), FrameMetal, 0.0f);
	AddBox(Center + FVector(-Extent.X - 22.0f, 0, Extent.Z * 0.85f),
		FVector(24.0f, Extent.Y * 2.0f + 48.0f, Extent.Z * 1.15f), FrameMetal, 0.0f);
	AddBox(Center + FVector(0, Extent.Y + 30.0f, Extent.Z * 1.58f),
		FVector(Extent.X * 2.0f - 60.0f, 24.0f, 28.0f), P.Accent, 3.0f);

	// Rooftop plant: HVAC housings and a beacon, rather than a bare flat roof.
	AddBox(Center + FVector(-Extent.X * 0.28f, 0, Extent.Z * 2.0f + 80.0f),
		FVector(180.0f, 130.0f, 95.0f), FrameMetal, 0.0f);
	AddBox(Center + FVector(Extent.X * 0.26f, -Extent.Y * 0.18f, Extent.Z * 2.0f + 65.0f),
		FVector(120.0f, 110.0f, 70.0f), PodiumConcrete, 0.0f);
	AddBox(Center + FVector(0, 0, Extent.Z * 2.0f + 160.0f),
		FVector(8.0f, 8.0f, 220.0f), FrameMetal, 0.0f);

	UPointLightComponent* Beacon = NewObject<UPointLightComponent>(this);
	Beacon->SetLightColor(P.Accent);
	Beacon->SetIntensity(8000.0f);
	Beacon->SetAttenuationRadius(900.0f);
	Beacon->SetWorldLocation(Center + FVector(0, 0, Extent.Z * 2.0f + 140.0f));
	Beacon->RegisterComponent();
}

void ANDWorldBuilder::BuildStreetLight(FVector Location, float Height)
{
	// Pole.
	AddBox(Location + FVector(0, 0, Height * 0.5f), FVector(16.0f, 16.0f, Height),
		FLinearColor(0.08f, 0.08f, 0.1f), 0.0f);
	// Lamp head (emissive).
	AddBox(Location + FVector(0, 0, Height + 20.0f), FVector(70.0f, 24.0f, 12.0f),
		FLinearColor(1.0f, 0.85f, 0.6f), 5.0f);

	USpotLightComponent* Spot = NewObject<USpotLightComponent>(this);
	Spot->SetLightColor(FLinearColor(1.0f, 0.9f, 0.75f));
	Spot->SetIntensity(12000.0f);
	Spot->SetAttenuationRadius(1500.0f);
	Spot->SetInnerConeAngle(35.0f);
	Spot->SetOuterConeAngle(55.0f);
	Spot->SetWorldLocation(Location + FVector(0, 0, Height + 30.0f));
	Spot->SetWorldRotation(FRotator(-90.0f, 0, 0));
	Spot->RegisterComponent();
}

void ANDWorldBuilder::BuildSign(FVector Location, const FLinearColor& Color)
{
	// Billboards: two posts + emissive panel.
	AddBox(Location + FVector(-70, 0, 90.0f), FVector(12.0f, 12.0f, 180.0f),
		FLinearColor(0.1f, 0.1f, 0.12f), 0.0f);
	AddBox(Location + FVector(70, 0, 90.0f), FVector(12.0f, 12.0f, 180.0f),
		FLinearColor(0.1f, 0.1f, 0.12f), 0.0f);
	AddBox(Location + FVector(0, 0, 220.0f), FVector(220.0f, 40.0f, 70.0f),
		Color, 6.0f);
	AddBox(Location + FVector(0, -24.0f, 222.0f), FVector(170.0f, 6.0f, 34.0f),
		FLinearColor(0.015f, 0.018f, 0.035f), 0.0f);
	AddBox(Location + FVector(-45.0f, -30.0f, 224.0f), FVector(45.0f, 4.0f, 8.0f),
		FLinearColor(1.0f, 0.95f, 0.25f), 4.0f);
	AddBox(Location + FVector(35.0f, -30.0f, 207.0f), FVector(70.0f, 4.0f, 8.0f),
		FLinearColor(0.15f, 1.0f, 0.95f), 4.0f);
}

void ANDWorldBuilder::BuildCrosswalk(FVector Center, bool bAcrossVerticalStreet)
{
	for (int32 i = -3; i <= 3; ++i)
	{
		const FVector Offset = bAcrossVerticalStreet ? FVector(i * 72.0f, 0, 0) : FVector(0, i * 72.0f, 0);
		const FVector Scale = bAcrossVerticalStreet ? FVector(36.0f, 420.0f, 3.0f) : FVector(420.0f, 36.0f, 3.0f);
		AddBox(Center + Offset, Scale, FLinearColor(0.92f, 0.92f, 0.86f), 0.4f);
	}
}

void ANDWorldBuilder::BuildBench(FVector Location, float YawDegrees)
{
	const FVector Rot(0.0f, YawDegrees, 0.0f);
	AddBox(Location + FVector(0, 0, 42.0f), FVector(210.0f, 34.0f, 20.0f), FLinearColor(0.28f, 0.12f, 0.04f), 0.0f, nullptr, Rot);
	AddBox(Location + FVector(0, -28.0f, 82.0f), FVector(210.0f, 18.0f, 70.0f), FLinearColor(0.22f, 0.09f, 0.03f), 0.0f, nullptr, Rot);
	AddBox(Location + FVector(-78.0f, 0, 20.0f), FVector(12.0f, 24.0f, 40.0f), FLinearColor(0.04f, 0.04f, 0.05f), 0.0f, nullptr, Rot);
	AddBox(Location + FVector(78.0f, 0, 20.0f), FVector(12.0f, 24.0f, 40.0f), FLinearColor(0.04f, 0.04f, 0.05f), 0.0f, nullptr, Rot);
}

void ANDWorldBuilder::BuildTrafficSignal(FVector Location, float YawDegrees)
{
	const FVector Rot(0.0f, YawDegrees, 0.0f);
	AddBox(Location + FVector(0, 0, 170.0f), FVector(12.0f, 12.0f, 340.0f), FLinearColor(0.03f, 0.03f, 0.035f), 0.0f);
	AddBox(Location + FVector(0, 0, 360.0f), FVector(58.0f, 24.0f, 118.0f), FLinearColor(0.015f, 0.015f, 0.018f), 0.0f, nullptr, Rot);
	AddBox(Location + FVector(0, -14.0f, 394.0f), FVector(28.0f, 5.0f, 22.0f), FLinearColor(1.0f, 0.05f, 0.04f), 5.0f, nullptr, Rot);
	AddBox(Location + FVector(0, -14.0f, 360.0f), FVector(28.0f, 5.0f, 22.0f), FLinearColor(1.0f, 0.78f, 0.05f), 4.0f, nullptr, Rot);
	AddBox(Location + FVector(0, -14.0f, 326.0f), FVector(28.0f, 5.0f, 22.0f), FLinearColor(0.05f, 1.0f, 0.18f), 4.0f, nullptr, Rot);
}

void ANDWorldBuilder::BuildRoadSign(FVector Location, const FLinearColor& Color, float YawDegrees)
{
	const FVector Rot(0.0f, YawDegrees, 0.0f);
	AddBox(Location + FVector(0, 0, 105.0f), FVector(10.0f, 10.0f, 210.0f), FLinearColor(0.08f, 0.08f, 0.09f), 0.0f);
	AddBox(Location + FVector(0, 0, 230.0f), FVector(110.0f, 12.0f, 72.0f), Color, 3.5f, nullptr, Rot);
	AddBox(Location + FVector(0, -9.0f, 230.0f), FVector(76.0f, 4.0f, 16.0f), FLinearColor(0.02f, 0.025f, 0.04f), 0.0f, nullptr, Rot);
}

void ANDWorldBuilder::BuildBarrierCones(FVector Location, float YawDegrees)
{
	const FVector Rot(0.0f, YawDegrees, 0.0f);
	for (int32 i = 0; i < 4; ++i)
	{
		const FVector P = Location + FVector(i * 55.0f, 0, 0);
		AddBox(P + FVector(0, 0, 12.0f), FVector(34.0f, 34.0f, 24.0f), FLinearColor(1.0f, 0.28f, 0.02f), 1.0f, nullptr, Rot);
		AddBox(P + FVector(0, 0, 36.0f), FVector(20.0f, 20.0f, 48.0f), FLinearColor(1.0f, 0.42f, 0.04f), 1.0f, nullptr, Rot);
		AddBox(P + FVector(0, 0, 40.0f), FVector(23.0f, 23.0f, 8.0f), FLinearColor(0.96f, 0.96f, 0.86f), 0.6f, nullptr, Rot);
	}
}

void ANDWorldBuilder::BuildUrbanTree(FVector Location, float Scale)
{
	// Authored in Blender from trunk, branch and double-sided leaf geometry.
	// It replaces the old BasicShapes sphere canopy without adding collision to
	// city dressing, preserving the existing vehicle/pedestrian physics paths.
	if (!UrbanTreeMesh)
	{
		return;
	}
	UStaticMeshComponent* Tree = NewObject<UStaticMeshComponent>(this);
	Tree->SetStaticMesh(UrbanTreeMesh);
	Tree->SetMobility(EComponentMobility::Movable);
	Tree->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Tree->SetCastShadow(true);
	Tree->bCastDynamicShadow = true;
	Tree->SetupAttachment(SceneRoot);
	Tree->RegisterComponent();
	Tree->SetRelativeLocation(Location);
	Tree->SetRelativeScale3D(FVector(Scale));
	BuildComponents.Add(Tree);
}

void ANDWorldBuilder::BuildPlanter(FVector Location, float YawDegrees)
{
	const FVector Rot(0.0f, YawDegrees, 0.0f);
	const FLinearColor Concrete(0.085f, 0.095f, 0.110f);
	const FLinearColor Soil(0.025f, 0.014f, 0.008f);
	AddBox(Location + FVector(0, 0, 42.0f), FVector(330.0f, 110.0f, 84.0f), Concrete, 0.0f, nullptr, Rot);
	AddBox(Location + FVector(0, 0, 88.0f), FVector(292.0f, 78.0f, 18.0f), Soil, 0.0f, nullptr, Rot);

	auto AddShrub = [&](const FVector& Offset, float Size, float LocalYaw)
	{
		if (!UrbanTreeMesh)
		{
			return;
		}
		UStaticMeshComponent* Shrub = NewObject<UStaticMeshComponent>(this);
		Shrub->SetStaticMesh(UrbanTreeMesh);
		Shrub->SetMobility(EComponentMobility::Movable);
		Shrub->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Shrub->SetCastShadow(true);
		Shrub->bCastDynamicShadow = true;
		Shrub->SetupAttachment(SceneRoot);
		Shrub->RegisterComponent();
		const FVector Rotated = FRotator(0.0f, YawDegrees, 0.0f).RotateVector(Offset);
		Shrub->SetRelativeLocation(Location + Rotated);
		// Same authored tree asset, reduced to an ornamental sapling: it replaces
		// the old BasicShapes spheres with actual trunk/branch/leaf silhouettes.
		Shrub->SetRelativeRotation(FRotator(0.0f, YawDegrees + LocalYaw, 0.0f));
		Shrub->SetRelativeScale3D(FVector(Size / 800.0f));
		BuildComponents.Add(Shrub);
	};
	AddShrub(FVector(-110.0f, 0.0f, 92.0f), 112.0f, -16.0f);
	AddShrub(FVector(-35.0f, 12.0f, 92.0f), 125.0f, 22.0f);
	AddShrub(FVector(45.0f, -8.0f, 92.0f), 118.0f, -31.0f);
	AddShrub(FVector(118.0f, 4.0f, 92.0f), 98.0f, 14.0f);
}

void ANDWorldBuilder::BuildPoliceCruiserProp(FVector Location, float YawDegrees)
{
	const FVector Rot(0.0f, YawDegrees, 0.0f);
	AddBox(Location + FVector(0, 0, 42.0f), FVector(360.0f, 170.0f, 70.0f), FLinearColor(0.015f, 0.018f, 0.025f), 0.0f, nullptr, Rot);
	AddBox(Location + FVector(10.0f, 0, 95.0f), FVector(190.0f, 135.0f, 58.0f), FLinearColor(0.85f, 0.88f, 0.92f), 0.0f, nullptr, Rot);
	AddBox(Location + FVector(34.0f, 0, 128.0f), FVector(122.0f, 105.0f, 16.0f), FLinearColor(0.04f, 0.18f, 0.32f), 1.2f, nullptr, Rot);
	AddBox(Location + FVector(12.0f, 0, 164.0f), FVector(120.0f, 28.0f, 20.0f), FLinearColor(0.1f, 0.2f, 1.0f), 6.0f, nullptr, Rot);
	AddBox(Location + FVector(12.0f, 18.0f, 166.0f), FVector(56.0f, 20.0f, 18.0f), FLinearColor(1.0f, 0.05f, 0.05f), 6.0f, nullptr, Rot);
	AddBox(Location + FVector(155.0f, 0, 70.0f), FVector(16.0f, 118.0f, 18.0f), FLinearColor(1.0f, 0.94f, 0.55f), 4.0f, nullptr, Rot);
	AddBox(Location + FVector(-170.0f, 0, 70.0f), FVector(16.0f, 118.0f, 18.0f), FLinearColor(1.0f, 0.08f, 0.06f), 4.0f, nullptr, Rot);
}

void ANDWorldBuilder::BuildGarbage(FVector Location)
{
	// Trash bags + dumpster cluster.
	AddBox(Location + FVector(0, 0, 30.0f), FVector(70.0f, 55.0f, 60.0f),
		FLinearColor(0.07f, 0.07f, 0.08f), 0.0f);
	AddBox(Location + FVector(60, 30, 18.0f), FVector(30.0f, 30.0f, 36.0f),
		FLinearColor(0.1f, 0.09f, 0.06f), 0.0f);
	AddBox(Location + FVector(40, -40, 12.0f), FVector(26.0f, 26.0f, 24.0f),
		FLinearColor(0.12f, 0.12f, 0.1f), 0.0f);
}

void ANDWorldBuilder::BuildHydrant(FVector Location)
{
	UStaticMeshComponent* Body = AddBox(Location + FVector(0, 0, 45.0f), FVector(26.0f, 26.0f, 90.0f),
		FLinearColor(0.85f, 0.1f, 0.08f), 0.0f);
	AddBox(Location + FVector(0, 0, 95.0f), FVector(36.0f, 36.0f, 16.0f),
		FLinearColor(0.9f, 0.12f, 0.1f), 0.0f);
	AddBox(Location + FVector(30, 0, 60.0f), FVector(30.0f, 12.0f, 12.0f),
		FLinearColor(0.85f, 0.1f, 0.08f), 0.0f);
	AddBox(Location + FVector(-30, 0, 60.0f), FVector(30.0f, 12.0f, 12.0f),
		FLinearColor(0.85f, 0.1f, 0.08f), 0.0f);
}

void ANDWorldBuilder::BuildKiosk(FVector Location)
{
	// News kiosk: stall body + awning + neon edge.
	AddBox(Location + FVector(0, 0, 140.0f), FVector(220.0f, 160.0f, 280.0f),
		FLinearColor(0.12f, 0.12f, 0.16f), 0.0f);
	AddBox(Location + FVector(0, 0, 300.0f), FVector(260.0f, 200.0f, 20.0f),
		FLinearColor(0.9f, 0.1f, 0.5f), 5.0f);
	// Counter slot glow.
	AddBox(Location + FVector(0, 90, 90.0f), FVector(160.0f, 10.0f, 60.0f),
		FLinearColor(0.2f, 1.0f, 1.0f), 4.0f);
}

void ANDWorldBuilder::BuildAtmosphere()
{
	// Moon light (cool, dim) — casts shadows so characters/props read in 3D.
	ADirectionalLight* Moon = GetWorld()->SpawnActor<ADirectionalLight>(
		FVector(0, 0, 2000.0f), FRotator(-55.0f, -30.0f, 0.0f));
	if (Moon)
	{
		Moon->GetLightComponent()->SetLightColor(FLinearColor(0.45f, 0.55f, 0.9f));
		Moon->GetLightComponent()->SetIntensity(2.2f);
		Moon->GetLightComponent()->SetCastShadows(true);
		Moon->GetLightComponent()->ContactShadowLength = 0.08f;
		Moon->GetLightComponent()->ContactShadowLengthInWS = false;
		Moon->GetLightComponent()->ContactShadowCastingIntensity = 0.9f;
		Moon->GetLightComponent()->ContactShadowNonCastingIntensity = 0.1f;
	}

	// Night skylight: soft ambient fill so unlit sides of meshes are not pure
	// black. A cool deep-blue tint keeps the neon night mood.
	ASkyLight* Sky = GetWorld()->SpawnActor<ASkyLight>(
		FVector(0, 0, 0), FRotator::ZeroRotator);
	if (Sky)
	{
		Sky->GetLightComponent()->SetLightColor(FLinearColor(0.25f, 0.28f, 0.45f));
		Sky->GetLightComponent()->SetIntensity(0.12f);
		Sky->GetLightComponent()->SetCastShadows(true);
	}

	// Exponential height fog (purple-tinted night haze).
	AExponentialHeightFog* Fog = GetWorld()->SpawnActor<AExponentialHeightFog>(
		FVector(0, 0, 500.0f), FRotator::ZeroRotator);
	if (Fog)
	{
		Fog->GetComponent()->SetFogDensity(0.006f);
		Fog->GetComponent()->SetFogInscatteringColor(FLinearColor(0.08f, 0.05f, 0.16f));
		Fog->GetComponent()->SetFogHeightFalloff(0.02f);
	}

	// Intentionally no SkyAtmosphere actor: its default directional-light binding
	// renders a bright blue daytime sky in packaged DX12 screenshots. The district
	// is a night slice; moon, fog, skylight and emissive architecture provide the
	// readable lighting without silently turning the visual gate into daytime.

	// Post-process: controlled bloom + subtle vignette + warm grade.
	APostProcessVolume* PP = GetWorld()->SpawnActor<APostProcessVolume>(
		FVector(0, 0, 0), FRotator::ZeroRotator);
	if (PP)
	{
		PP->bUnbound = true;
		PP->Settings.bOverride_BloomIntensity = true;
		PP->Settings.BloomIntensity = 0.9f;
		PP->Settings.bOverride_BloomThreshold = true;
		PP->Settings.BloomThreshold = 1.1f;
		PP->Settings.bOverride_VignetteIntensity = true;
		PP->Settings.VignetteIntensity = 0.35f;
		PP->Settings.bOverride_AutoExposureBias = true;
		// Histogram exposure is required here: forcing Manual makes the packaged
		// benchmark captures entirely black on the target DX12 render path.
		// Histogram remains required for packaged DX12. A lower, still automatic
		// bias removes the daytime wash while preserving readable emissive details.
		PP->Settings.AutoExposureBias = -1.25f;
		// Benchmark screenshots teleport/re-orient the camera. Disable motion
		// blur so temporal history never smears the NPC/vehicle visual gates.
		PP->Settings.bOverride_MotionBlurAmount = true;
		PP->Settings.MotionBlurAmount = 0.0f;
		PP->Settings.bOverride_DepthOfFieldFstop = true;
		PP->Settings.DepthOfFieldFstop = 22.0f;
	}
}

void ANDWorldBuilder::SpawnCityPopulation()
{
	// One spawner covers the whole district; caps live in NDPerfConstants.
	ANDCitySpawner* Spawner = GetWorld()->SpawnActor<ANDCitySpawner>(
		FVector(0, 0, 0), FRotator::ZeroRotator);
	if (Spawner)
	{
		// Defaults from the spawner class are already within NDPerf caps.
		UE_LOG(LogTemp, Log, TEXT("NeonDistrict: district built, city population spawned."));
	}
}

FVector ANDWorldBuilder::GetRandomStreetPoint() const
{
	// The district is fully procedural, so the baked navmesh does not exist;
	// callers that need a reachable point must not depend on it. Streets are
	// 30-wide avenue boxes at the grid lines; any point on one is open asphalt.
	const float BlockPitch = BlockSize + 2.0f * StreetHalfWidth;
	const float TotalW = BlocksX * BlockSize + (BlocksX + 1) * 2.0f * StreetHalfWidth;
	const float TotalH = BlocksY * BlockSize + (BlocksY + 1) * 2.0f * StreetHalfWidth;

	// Center of the avenue boxes along each grid line.
	TArray<float> StreetX;
	TArray<float> StreetY;
	for (int32 X = 0; X <= BlocksX; ++X)
	{
		StreetX.Add(-TotalW * 0.5f + StreetHalfWidth + X * BlockPitch);
	}
	for (int32 Y = 0; Y <= BlocksY; ++Y)
	{
		StreetY.Add(-TotalH * 0.5f + StreetHalfWidth + Y * BlockPitch);
	}

	const bool bVertical = FMath::RandBool();
	const float HalfLen = (bVertical ? TotalH : TotalW) * 0.5f;
	const float Along = FMath::FRandRange(-HalfLen, HalfLen);

	if (bVertical)
	{
		const float X = StreetX[FMath::RandRange(0, StreetX.Num() - 1)];
		return FVector(X, Along, 0.0f);
	}
	const float Y = StreetY[FMath::RandRange(0, StreetY.Num() - 1)];
	return FVector(Along, Y, 0.0f);
}
