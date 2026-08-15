// Copyright Neon District Sandbox. Public benchmark repo — original content only.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "NDWorldSubsystem.generated.h"

/**
 * Auto-builds the Neon District on world begin: in any level that is not the
 * main menu, spawns the procedural world builder + city population. This keeps
 * the repo runnable with nothing more than an empty level saved as ND_City.
 */
UCLASS()
class NEONDISTRICT_API UNDWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
	bool bDistrictBuilt = false;
};
