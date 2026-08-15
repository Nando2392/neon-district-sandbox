// Copyright Neon District Sandbox. Public benchmark repo — original content only.
// Editor-only commandlet: creates the two truly-empty maps (0 actors).
// The whole city is built at runtime by UNDWorldSubsystem / ANDWorldBuilder.

#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "NDCreateMapsCommandlet.generated.h"

UCLASS()
class UNDCreateMapsCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	virtual int32 Main(const FString& Params) override;
};
