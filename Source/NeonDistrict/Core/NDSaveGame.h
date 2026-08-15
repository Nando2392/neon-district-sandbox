// Copyright Neon District Sandbox. Public benchmark repo — original content only.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "NDSaveGame.generated.h"

/**
 * Versioned save snapshot — data only, never engine object references.
 * v1: mission progress + player location + wanted level + settings.
 */
UCLASS()
class NEONDISTRICT_API UNDSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UNDSaveGame();

	UPROPERTY(VisibleAnywhere, Category = "Save")
	int32 SaveVersion = 1;

	// Mission progress: 0 = not started, 1 = accepted, 2 = package picked, 3 = delivered/completed
	UPROPERTY(VisibleAnywhere, Category = "Save|Mission")
	int32 MissionStage = 0;

	UPROPERTY(VisibleAnywhere, Category = "Save|Mission")
	FString MissionNPCName = TEXT("");

	UPROPERTY(VisibleAnywhere, Category = "Save|Player")
	FVector PlayerLocation = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, Category = "Save|Player")
	float PlayerYaw = 0.0f;

	UPROPERTY(VisibleAnywhere, Category = "Save|Wanted")
	int32 WantedLevel = 0;

	UPROPERTY(VisibleAnywhere, Category = "Save|Audio")
	float MasterVolume = 0.85f;

	UPROPERTY(VisibleAnywhere, Category = "Save|Audio")
	float MusicVolume = 0.70f;

	UPROPERTY(VisibleAnywhere, Category = "Save|Audio")
	float SfxVolume = 1.0f;
};
