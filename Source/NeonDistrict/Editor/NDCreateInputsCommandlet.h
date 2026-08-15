// Copyright Neon District Sandbox. Public benchmark repo — original content only

#pragma once

#include "Commandlets/Commandlet.h"
#include "NDCreateInputsCommandlet.generated.h"

/**
 * Commandlet to generate InputAction assets for Enhanced Input
 * Run with: NeonDistrictEditor.exe -run=CreateInputAssets
 */
UCLASS()
class NEONDISTRICT_API UNDCreateInputsCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	virtual int32 Main(const FString& Params) override;
};