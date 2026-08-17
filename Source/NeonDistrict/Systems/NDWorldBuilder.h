// Copyright Neon District Sandbox. Public benchmark repo — original content only.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NDWorldBuilder.generated.h"

class UMaterial;
class UMaterialInstanceDynamic;
class USceneComponent;
class UStaticMesh;
class UTexture2D;

/**
 * Procedural city builder. Constructs the whole Neon District on BeginPlay:
 * street grid (2x2 blocks), sidewalks, buildings with emissive neon facades and
 * window grids, street lights, signs, garbage/decor props, alleyways, fog, moon,
 * bloom post-process, and the runtime city spawner. Zero .umap assets needed —
 * the level can be an empty world and the district appears anyway.
 *
 * Visual intent: synthwave/neon-night palette (magenta/cyan/violet) with
 * controlled bloom and volumetric-style fog, NOT default-template grey boxes.
 */
UCLASS()
class NEONDISTRICT_API ANDWorldBuilder : public AActor
{
	GENERATED_BODY()

public:
	ANDWorldBuilder();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	/** Street half-width (each avenue is 2*this wide). */
	UPROPERTY(EditAnywhere, Category = "District")
	float StreetHalfWidth = 600.0f;

	/** Block footprint size. */
	UPROPERTY(EditAnywhere, Category = "District")
	float BlockSize = 2600.0f;

	UPROPERTY(EditAnywhere, Category = "District")
	int32 BlocksX = 2;

	UPROPERTY(EditAnywhere, Category = "District")
	int32 BlocksY = 2;

	/** Building footprint range per block edge. */
	UPROPERTY(EditAnywhere, Category = "District")
	float MinBuildingWidth = 400.0f;

	UPROPERTY(EditAnywhere, Category = "District")
	float MaxBuildingWidth = 650.0f;

	UPROPERTY(EditAnywhere, Category = "District")
	float MinBuildingHeight = 600.0f;

	UPROPERTY(EditAnywhere, Category = "District")
	float MaxBuildingHeight = 1200.0f;

private:
	void BuildDistrict();
	void BuildStreet(FVector Center, bool bIsVertical);
	void BuildBlock(int32 BX, int32 BY);
	void BuildBuilding(FVector Center, FVector Extent, int32 PaletteIndex);
	void BuildStreetLight(FVector Location, float Height);
	void BuildSign(FVector Location, const FLinearColor& Color);
	void BuildCrosswalk(FVector Center, bool bAcrossVerticalStreet);
	void BuildBench(FVector Location, float YawDegrees);
	void BuildTrafficSignal(FVector Location, float YawDegrees);
	void BuildRoadSign(FVector Location, const FLinearColor& Color, float YawDegrees);
	void BuildBarrierCones(FVector Location, float YawDegrees);
	void BuildUrbanBackdrop();
	void BuildHeroStreetClutter();
	void BuildUrbanTree(FVector Location, float Scale = 1.0f);
	void BuildPlanter(FVector Location, float YawDegrees = 0.0f);
	void BuildPoliceCruiserProp(FVector Location, float YawDegrees);
	void BuildGarbage(FVector Location);
	void BuildHydrant(FVector Location);
	void BuildKiosk(FVector Location);
	void BuildAtmosphere();
	void SpawnCityPopulation();

public:
	/**
	 * Deterministic street point for spawning/benchmarks. The district grid is
	 * fully procedural, so the navmesh (baked into .umap levels) does not exist
	 * here; callers that need a reachable point must not depend on it. Returns a
	 * location on an avenue surface (Z=0 = asphalt top), away from block facades.
	 */
	UFUNCTION(BlueprintCallable, Category = "District")
	FVector GetRandomStreetPoint() const;

	// Materials
	UMaterial* CreateNeonMaterial();
	UMaterialInstanceDynamic* MakeMID(UMaterial* Base, const FLinearColor& Color, float EmissiveStrength);
	UTexture2D* CreateFacadeTexture(const FLinearColor& WallColor, const FLinearColor& LitWindowColor,
		int32 Seed, int32 Width = 192, int32 Height = 256);
	UMaterial* NeonMat = nullptr;
	UMaterial* MatteMat = nullptr;
	// CDO hard reference: keeps the locally authored asphalt asset reachable by
	// the cooker even though streets are constructed dynamically at runtime.
	UPROPERTY()
	TObjectPtr<UMaterial> AsphaltMat = nullptr;
	// Separate concrete paving surface; retained as a CDO reference for cooking.
	UPROPERTY()
	TObjectPtr<UMaterial> SidewalkMat = nullptr;
	// Original Blender tree; a hard reference guarantees packaged cooking.
	UPROPERTY()
	TObjectPtr<UStaticMesh> UrbanTreeMesh = nullptr;

	UStaticMeshComponent* AddBox(FVector Location, FVector Scale, const FLinearColor& Color, float Emissive,
		UMaterial* MaterialOverride = nullptr, FVector RelativeRotation = FVector::ZeroVector);

	UPROPERTY()
	TObjectPtr<USceneComponent> SceneRoot = nullptr;

	/** Palette: night-neon building colors (base, accent). */
	struct FNeonPalette
	{
		FLinearColor Base;
		FLinearColor Accent;
	};
	TArray<FNeonPalette> Palette;

	TArray<TObjectPtr<UStaticMeshComponent>> BuildComponents;

	// Runtime-created facade textures must stay referenced for the lifetime of
	// the district; otherwise GC can remove a texture still bound to a MID.
	UPROPERTY()
	TArray<TObjectPtr<UTexture2D>> RuntimeFacadeTextures;
};
