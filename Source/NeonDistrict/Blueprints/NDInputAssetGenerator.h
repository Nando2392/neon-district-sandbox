// Copyright Neon District Sandbox. Public benchmark repo — original content only

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "NDInputAssetGenerator.generated.h"

/**
 * Blueprint Function Library to generate Enhanced Input assets
 * Run GenerateAllInputAssets() once from the editor to create InputActions and MappingContext
 */
UCLASS()
class NEONDISTRICT_API UNDInputAssetGenerator : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Generate all InputAction assets and the InputMappingContext */
	UFUNCTION(CallInEditor, Category = "Input Assets")
	static void GenerateAllInputAssets();

private:
	/** Create a single InputAction asset */
	static UObject* CreateInputActionAsset(const FString& Name);

	/** Create the InputMappingContext asset */
	static UObject* CreateMappingContextAsset();
};