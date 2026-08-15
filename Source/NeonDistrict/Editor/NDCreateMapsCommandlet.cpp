// Copyright Neon District Sandbox. Public benchmark repo — original content only.

#include "Editor/NDCreateMapsCommandlet.h"

#if WITH_EDITOR
#include "FileHelpers.h"
#include "Engine/World.h"

int32 UNDCreateMapsCommandlet::Main(const FString& Params)
{
	const TArray<FString> Maps = { TEXT("ND_MainMenu"), TEXT("ND_City") };
	int32 Failures = 0;

	for (const FString& MapName : Maps)
	{
		const FString PackagePath = FString::Printf(TEXT("/Game/Maps/%s"), *MapName);

		UWorld* World = UEditorLoadingAndSavingUtils::NewBlankMap(false);
		if (!World)
		{
			UE_LOG(LogTemp, Error, TEXT("NDCreateMaps: NewBlankMap failed for %s"), *MapName);
			++Failures;
			continue;
		}

		// NewBlankMap clones the engine template map, so the world's package
		// still carries the template name (/Engine/Maps/Templates/Template_Default).
		// Rename package + world to the unique target name BEFORE saving, or the
		// AssetManager registers every map under the same PrimaryAssetID and the
		// cooker silently drops them (duplicate PrimaryAssetID warning, zero
		// cooked .umap, game boots into OpenWorld instead of our map).
		UPackage* WorldPackage = World->GetOutermost();
		if (WorldPackage && WorldPackage->GetName() != PackagePath)
		{
			const EObjectFlags RenameFlags = static_cast<EObjectFlags>(REN_DontCreateRedirectors | REN_NonTransactional);
			WorldPackage->Rename(*PackagePath, nullptr, RenameFlags);
			World->Rename(*MapName, WorldPackage, RenameFlags);
			UE_LOG(LogTemp, Display, TEXT("NDCreateMaps: renamed world package to %s"), *PackagePath);
		}

		if (!UEditorLoadingAndSavingUtils::SaveMap(World, PackagePath))
		{
			UE_LOG(LogTemp, Error, TEXT("NDCreateMaps: SaveMap failed for %s"), *MapName);
			++Failures;
			continue;
		}

		UE_LOG(LogTemp, Display, TEXT("NDCreateMaps: %s saved OK (blank map)"), *PackagePath);
	}

	return Failures == 0 ? 0 : 1;
}
#else
// Game target (WITH_EDITOR=0): commandlet is editor-only; the stub keeps the
// class linkable in the shipped binary (it can never be invoked at runtime).
int32 UNDCreateMapsCommandlet::Main(const FString& Params)
{
	return 0;
}
#endif
