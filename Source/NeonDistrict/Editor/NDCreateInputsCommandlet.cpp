// Copyright Neon District Sandbox. Public benchmark repo — original content only

// Commandlet to generate InputAction assets for Enhanced Input
// Run with: NeonDistrictEditor.exe -run=CreateInputAssets

#include "NDCreateInputsCommandlet.h"

#include "EngineUtils.h"
#include "AssetToolsModule.h"
#include "ObjectTools.h"
#include "InputAction.h"
#include "InputMappingContext.h"

#include "Engine/Engine.h"
#include "Misc/Paths.h"
#include "Misc/ScopedSlowTask.h"
#include "Packages.h"
#include "UnrealEditor.h"

DEFINE_LOG_CATEGORY_STATIC(LogNDCreateInputAssets, Log, All);

int32 UNDCreateInputsCommandlet::Main(const FString& Params)
{
	// Only allowed in editor
	if (!GIsEditor || !GEngine)
	{
		UE_LOG(LogNDCreateInputAssets, Warning, TEXT("This commandlet can only run in the editor."));
		return 1;
	}

	const FString PackageRoot = FPackageName::FilenameToLongPackageName(*FPaths::ProjectContentDir());
	
	// Create InputAction assets
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

	FText AssocName;
	TMap<FName, FSoftObjectPtr> CreatedActions;

	for (const FString& ActionName : ActionNames)
	{
		FString PackagePath = FString::Printf(TEXT("/Game/Input/Actions/%s"), *ActionName);
		FName PackageName(*PackagePath);
		
		FAssetToolsModule& AssetToolsModule = FModuleManager::Get().GetModuleChecked<FAssetToolsModule>("AssetTools");
		TArray<FName> CreatedAssets;
		
		auto AssetInfo = AssetToolsModule.Get().CreateAsset(
			FName(*ActionName),
			FName(*ActionName),
			UInputAction::StaticClass(),
			nullptr
		);
		
		if (AssetInfo.IsValid())
		{
			UE_LOG(LogNDCreateInputAssets, Log, TEXT("Created InputAction: %s"), *ActionName);
			CreatedActions.Add(FName(*ActionName), AssetInfo.Asset);
		}
	}

	// Create InputMappingContext
	{
		FString PackagePath = TEXT("/Game/Input/Mappings/ND_DefaultContext");
		FAssetToolsModule& AssetToolsModule = FModuleManager::Get().GetModuleChecked<FAssetToolsModule>("AssetTools");
		
		auto AssetInfo = AssetToolsModule.Get().CreateAsset(
			FName(TEXT("ND_DefaultContext")),
			FName(TEXT("ND_DefaultContext")),
			UInputMappingContext::StaticClass(),
			nullptr
		);
		
		if (AssetInfo.IsValid())
		{
			UE_LOG(LogNDCreateInputAssets, Log, TEXT("Created InputMappingContext: ND_DefaultContext"));
		}
	}

	UE_LOG(LogNDCreateInputAssets, Log, TEXT("Input asset creation complete. Please save and re-cook."));
	return 0;
}