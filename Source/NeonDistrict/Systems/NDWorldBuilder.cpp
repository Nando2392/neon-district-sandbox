// Copyright Neon District Sandbox. Public benchmark repo — original content only.

#include "Systems/NDWorldBuilder.h"
#include "AI/NDCitySpawner.h"
#include "Core/NDPerfConstants.h"

#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/PostProcessComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
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
	// IMPORTANT: WITH_EDITOR is true for the UnrealEditor binary even when
	// launched with `-game` (standalone), but the MaterialEditor module is not
	// loaded in game mode, so UMaterialEditingLibrary calls crash. Gate on the
	// runtime mode: FApp::IsGame() is true for standalone `-game`; PIE in the
	// editor keeps the full editor path.
#if WITH_EDITOR
	if (!FApp::IsGame())
	{
		// NeonMat: BaseColor <- NeonColor; Emissive <- NeonColor * EmissiveStrength
		// (Material pin wiring is editor-only via UMaterialEditingLibrary in UE 5.8.)
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
	// Standalone game (-game) / packaged build: no procedural material graph
	// available; fall back to the engine's basic shape material. Parameters are
	// no-ops, so buildings render neutral gray — documented limitation, not hidden.
	return LoadObject<UMaterial>(nullptr,
		TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
}

UMaterialInstanceDynamic* ANDWorldBuilder::MakeMID(UMaterial* Base, const FLinearColor& Color, float EmissiveStrength)
{
	UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(Base, this);
	MID->SetVectorParameterValue(TEXT("NeonColor"), Color);
	MID->SetScalarParameterValue(TEXT("EmissiveStrength"), EmissiveStrength);
	return MID;
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
	Comp->SetRelativeScale3D(Scale);
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
		MatteMat = CreateNeonMaterial(); // reuse; strength 0 = matte
	}

	const float TotalW = BlocksX * BlockSize + (BlocksX + 1) * 2.0f * StreetHalfWidth;
	const float TotalH = BlocksY * BlockSize + (BlocksY + 1) * 2.0f * StreetHalfWidth;
	const FVector DistrictCenter = FVector::ZeroVector;

	// Ground (asphalt) — one big matte slab.
	AddBox(DistrictCenter + FVector(0, 0, -10), FVector(TotalW, TotalH, 10),
		FLinearColor(0.03f, 0.03f, 0.05f), 0.0f);

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
	BuildGarbage(FVector(StreetHalfWidth + 260.0f, -StreetHalfWidth - 260.0f, 0));
	BuildGarbage(FVector(-StreetHalfWidth - 260.0f, StreetHalfWidth + 260.0f, 0));
	BuildHydrant(FVector(StreetHalfWidth + 180.0f, StreetHalfWidth + 420.0f, 0));

	BuildAtmosphere();
	SpawnCityPopulation();
}

void ANDWorldBuilder::BuildStreet(FVector Center, bool bIsVertical)
{
	const FLinearColor Asphalt(0.02f, 0.02f, 0.035f);
	const FLinearColor LaneLine(0.95f, 0.90f, 0.60f);

	const float StreetLen = (BlocksX + 1) * BlockSize + (BlocksX + 2) * 2.0f * StreetHalfWidth;
	const float StreetWide = 2.0f * StreetHalfWidth;

	if (bIsVertical)
	{
		AddBox(Center + FVector(0, 0, -4), FVector(30.0f, StreetLen, 8.0f), Asphalt, 0.0f);
		// Center lane marking (dashed look via thin emissive strip).
		AddBox(Center + FVector(0, 0, -1), FVector(16.0f, StreetLen, 2.0f), LaneLine, 2.5f);
	}
	else
	{
		AddBox(Center + FVector(0, 0, -4), FVector(StreetLen, 30.0f, 8.0f), Asphalt, 0.0f);
		AddBox(Center + FVector(0, 0, -1), FVector(StreetLen, 16.0f, 2.0f), LaneLine, 2.5f);
	}
}

void ANDWorldBuilder::BuildBlock(int32 BX, int32 BY)
{
	const float TotalW = BlocksX * BlockSize + (BlocksX + 1) * 2.0f * StreetHalfWidth;
	const float TotalH = BlocksY * BlockSize + (BlocksY + 1) * 2.0f * StreetHalfWidth;
	const float BlockPitch = BlockSize + 2.0f * StreetHalfWidth;

	const FVector BlockOrigin(
		-TotalW * 0.5f + StreetHalfWidth + BX * BlockPitch,
		-TotalH * 0.5f + StreetHalfWidth + BY * BlockPitch,
		0.0f);

	// Sidewalk slab around the block.
	AddBox(BlockOrigin + FVector(BlockSize * 0.5f, BlockSize * 0.5f, 8.0f),
		FVector(BlockSize + 160.0f, BlockSize + 160.0f, 16.0f),
		FLinearColor(0.16f, 0.16f, 0.20f), 0.0f);

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

	// Facade box.
	AddBox(Center + FVector(0, 0, Extent.Z), FVector(Extent.X * 2.0f, Extent.Y * 2.0f, Extent.Z * 2.0f),
		P.Base, 0.0f);

	// Neon accent strips on the facade (vertical + horizontal bands).
	const float StripDepth = 12.0f;
	AddBox(Center + FVector(0, Extent.Y + StripDepth, Extent.Z * 0.4f),
		FVector(Extent.X * 2.0f - 40.0f, StripDepth, 50.0f), P.Accent, 6.0f);
	AddBox(Center + FVector(0, -Extent.Y - StripDepth, Extent.Z * 0.7f),
		FVector(Extent.X * 2.0f - 40.0f, StripDepth, 40.0f), P.Accent, 6.0f);
	AddBox(Center + FVector(Extent.X + StripDepth, 0, Extent.Z * 0.5f),
		FVector(StripDepth, Extent.Y * 2.0f - 40.0f, 46.0f), P.Accent, 6.0f);
	AddBox(Center + FVector(-Extent.X - StripDepth, 0, Extent.Z * 0.3f),
		FVector(StripDepth, Extent.Y * 2.0f - 40.0f, 36.0f), P.Accent, 6.0f);

	// Window grid: emissive warm windows on the two main faces.
	const float WindowW = 70.0f;
	const float WindowH = 110.0f;
	const float GapX = 130.0f;
	const float GapZ = 180.0f;
	const int32 Cols = FMath::Max(1, FMath::FloorToInt(Extent.X * 2.0f / GapX));
	const int32 Rows = FMath::Max(1, FMath::FloorToInt(Extent.Z * 2.0f / GapZ) - 1);

	for (int32 C = 0; C < Cols; ++C)
	{
		for (int32 R = 0; R < Rows; ++R)
		{
			const float WX = Center.X - Extent.X + GapX * 0.5f + C * GapX;
			const float WZ = 120.0f + R * GapZ;
			const FLinearColor WindowColor = (FMath::RandBool())
				? FLinearColor(1.0f, 0.95f, 0.6f)
				: FLinearColor(0.9f, 0.5f, 0.2f);

			AddBox(FVector(WX, Center.Y + Extent.Y + 4.0f, WZ),
				FVector(WindowW, 8.0f, WindowH), WindowColor, 3.0f);
			AddBox(FVector(WX, Center.Y - Extent.Y - 4.0f, WZ),
				FVector(WindowW, 8.0f, WindowH), WindowColor, 3.0f);
		}
	}

	// Rooftop antenna with blinking beacon light.
	AddBox(Center + FVector(0, 0, Extent.Z * 2.0f + 60.0f),
		FVector(8.0f, 8.0f, 120.0f), FLinearColor(0.3f, 0.3f, 0.35f), 0.0f);

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
	// Moon light (cool, dim).
	ADirectionalLight* Moon = GetWorld()->SpawnActor<ADirectionalLight>(
		FVector(0, 0, 2000.0f), FRotator(-55.0f, -30.0f, 0.0f));
	if (Moon)
	{
		Moon->GetLightComponent()->SetLightColor(FLinearColor(0.45f, 0.55f, 0.9f));
		Moon->GetLightComponent()->SetIntensity(1.2f);
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

	// Sky atmosphere for a night skyline.
	ASkyAtmosphere* Sky = GetWorld()->SpawnActor<ASkyAtmosphere>(
		FVector(0, 0, 0), FRotator::ZeroRotator);
	if (Sky)
	{
		Sky->GetComponent()->SetAtmosphereHeight(100.0f);
	}

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
		PP->Settings.AutoExposureBias = -0.4f;
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
