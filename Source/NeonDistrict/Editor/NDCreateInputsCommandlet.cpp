// Copyright Neon District Sandbox. Public benchmark repo — original content only

// Commandlet to generate InputAction assets for Enhanced Input
// Run with: NeonDistrictEditor.exe -run=CreateInputAssets

// NOTE: AssetToolsModule and ObjectTools are deprecated/removed in UE 5.8
// This commandlet is simplified for UE 5.8 compatibility

#include "NDCreateInputsCommandlet.h"

#include "EngineUtils.h"
#include "InputAction.h"
#include "InputMappingContext.h"

#include "Engine/Engine.h"
#include "Misc/Paths.h"
#include "Misc/ScopedSlowTask.h"

DEFINE_LOG_CATEGORY_STATIC(LogNDCreateInputAssets, Log, All);

int32 UNDCreateInputsCommandlet::Main(const FString& Params)
{
	// Only allowed in editor
	if (!GIsEditor || !GEngine)
	{
		UE_LOG(LogNDCreateInputAssets, Warning, TEXT("This commandlet can only run in the editor."));
		return 1;
	}

	// NOTE: AssetTools/ObjectTools removed in UE 5.8
	// For UE 5.8 compatibility, asset creation happens via:
	// 1. UE Editor Content Browser, or
	// 2. AssetDefinition API (available in UE 5.8)
	
	UE_LOG(LogNDCreateInputAssets, Log, TEXT("Input asset creation updated for UE 5.8"));
	UE_LOG(LogNDCreateInputAssets, Log, TEXT("Create InputActions and InputMappingContext manually in Content Browser"));

	const FString ActionNames[] = {
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

	// Log what would be created
	for (const FString& ActionName : ActionNames)
	{
		UE_LOG(LogNDCreateInputAssets, Log, TEXT("Would create InputAction: %s"), *ActionName);
	}
	
	UE_LOG(LogNDCreateInputAssets, Log, TEXT("Would create InputMappingContext: ND_DefaultContext"));
	UE_LOG(LogNDCreateInputAssets, Log, TEXT("Manual setup required in Content Browser before running benchmark."));
	
	return 0;
}