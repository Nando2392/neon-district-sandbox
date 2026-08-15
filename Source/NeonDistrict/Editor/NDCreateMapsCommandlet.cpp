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
#endif
