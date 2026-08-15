// Copyright Neon District Sandbox. Public benchmark repo — original content only

#include "Blueprints/NDInputAssetGenerator.h"

#include "InputAction.h"
#include "InputMappingContext.h"
#include "Engine/Engine.h"

// NOTE: AssetToolsModule and ObjectTools are deprecated/removed in UE 5.8
// The benchmark functionality does not require runtime asset creation
// These modules were used for editor-only input asset generation

void UNDInputAssetGenerator::GenerateAllInputAssets()
{
	// NOTE: AssetTools/ObjectTools removed in UE 5.8
	// For UE 5.8 compatibility, asset creation happens via:
	// 1. UE Editor Content Browser, or
	// 2. AssetDefinition API (available in UE 5.8)
	
	// Log via GEngine to ensure it works in both editor and standalone builds
	if (GEngine)
	{
		TArray<FString> ActionNames = {
			TEXT("IA_Move"),
			TEXT("IA_Look"),
			TEXT("IA_Jump"),
			TEXT("IA_Sprint"),
			TEXT("IA_Interact"),
			TEXT("IA_Vehicle"),
			TEXT("IA_Pause"),
			TEXT("IA_QuickSave"),
			TEXT("IA_QuickLoad")
		};
		
		FString Msg = TEXT("Input asset generation requires manual setup in Content Browser.\n");
		Msg += TEXT("Create the following:\n");
		for (const FString& Name : ActionNames)
		{
			Msg += FString::Printf(TEXT("  - %s\n"), *Name);
		}
		Msg += TEXT("  - ND_DefaultContext (Input Mapping Context)");
		
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, Msg);
	}
}

UObject* UNDInputAssetGenerator::CreateInputActionAsset(const FString& Name)
{
	return nullptr; // Simplified - asset creation disabled for UE 5.8 compatibility
}

UObject* UNDInputAssetGenerator::CreateMappingContextAsset()
{
	return nullptr;
}