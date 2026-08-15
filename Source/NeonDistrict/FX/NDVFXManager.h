// Copyright Neon District Sandbox. Public benchmark repo — original content only.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "NDVFXManager.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;

UENUM(BlueprintType)
enum class ENDFXType : uint8
{
	Smoke         UMETA(DisplayName = "Smoke"),
	ImpactSparks  UMETA(DisplayName = "Impact Sparks"),
	Dust          UMETA(DisplayName = "Dust"),
	LightPulse    UMETA(DisplayName = "Light Pulse"),
	Impact        UMETA(DisplayName = "Impact")
};

/**
 * Niagara FX manager with per-type pooling and an explicit actor cap
 * (NDPerf::MaxFXActors). Spawn is event-driven only — never from Tick.
 * Editor assigns Niagara systems to each type via soft paths; zero assets = no-op.
 */
UCLASS()
class NEONDISTRICT_API UNDVFXManager : public UObject
{
	GENERATED_BODY()

public:
	void SpawnFX(ENDFXType Type, const FVector& Location, const FRotator& Rotation, float Scale = 1.0f);

	// Editor-configurable Niagara systems (may be empty).
	UPROPERTY(EditAnywhere, Category = "FX|Systems")
	TSoftObjectPtr<UNiagaraSystem> SmokeSystem;

	UPROPERTY(EditAnywhere, Category = "FX|Systems")
	TSoftObjectPtr<UNiagaraSystem> ImpactSparksSystem;

	UPROPERTY(EditAnywhere, Category = "FX|Systems")
	TSoftObjectPtr<UNiagaraSystem> DustSystem;

	UPROPERTY(EditAnywhere, Category = "FX|Systems")
	TSoftObjectPtr<UNiagaraSystem> LightPulseSystem;

	UPROPERTY(EditAnywhere, Category = "FX|Systems")
	TSoftObjectPtr<UNiagaraSystem> ImpactSystem;

private:
	UNiagaraSystem* ResolveSystem(ENDFXType Type);
	UNiagaraComponent* AcquireFromPool(ENDFXType Type);

	TMap<ENDFXType, TArray<TObjectPtr<UNiagaraComponent>>> Pool;
	TMap<ENDFXType, int32> PoolCursor;
	TMap<ENDFXType, float> LastSpawnTime;
	int32 TotalActive = 0;
};
