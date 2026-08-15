// Copyright Neon District Sandbox. Public benchmark repo — original content only.

#include "FX/NDVFXManager.h"
#include "Core/NDPerfConstants.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Engine/World.h"
#include "UObject/SoftObjectPtr.h"

void UNDVFXManager::SpawnFX(ENDFXType Type, const FVector& Location, const FRotator& Rotation, float Scale)
{
	UNiagaraSystem* System = ResolveSystem(Type);
	if (!System)
	{
		return;
	}

	// Per-type cooldown: never more than ~4 bursts per second per type.
	const float Now = GetWorld()->GetTimeSeconds();
	float& Last = LastSpawnTime.FindOrAdd(Type);
	if (Now - Last < 0.25f)
	{
		return;
	}
	Last = Now;

	UNiagaraComponent* Component = AcquireFromPool(Type);
	if (!Component)
	{
		return;
	}

	Component->SetAsset(System);
	Component->SetWorldLocationAndRotation(Location, Rotation);
	Component->SetRelativeScale3D(FVector(Scale));
	Component->Activate(true);
	Component->SetAutoActivate(false); // we control activation explicitly
}

UNiagaraSystem* UNDVFXManager::ResolveSystem(ENDFXType Type)
{
	TSoftObjectPtr<UNiagaraSystem>* Soft = nullptr;
	switch (Type)
	{
	case ENDFXType::Smoke:        Soft = &SmokeSystem; break;
	case ENDFXType::ImpactSparks: Soft = &ImpactSparksSystem; break;
	case ENDFXType::Dust:         Soft = &DustSystem; break;
	case ENDFXType::LightPulse:   Soft = &LightPulseSystem; break;
	case ENDFXType::Impact:       Soft = &ImpactSystem; break;
	}
	if (!Soft || Soft->IsNull())
	{
		return nullptr;
	}
	return Soft->LoadSynchronous();
}

UNiagaraComponent* UNDVFXManager::AcquireFromPool(ENDFXType Type)
{
	TArray<TObjectPtr<UNiagaraComponent>>& Bucket = Pool.FindOrAdd(Type);
	int32& Cursor = PoolCursor.FindOrAdd(Type);

	if (Bucket.Num() < NDPerf::MaxFXActors)
	{
		UNiagaraComponent* NewComponent = NewObject<UNiagaraComponent>(GetTransientPackage());
		NewComponent->bAutoActivate = false;
		NewComponent->RegisterComponent();
		Bucket.Add(NewComponent);
		++TotalActive;
		return NewComponent;
	}

	// Cap reached: recycle the oldest in this bucket (round-robin).
	Cursor = (Cursor + 1) % Bucket.Num();
	UNiagaraComponent* Recycled = Bucket[Cursor];
	if (Recycled)
	{
		Recycled->Deactivate();
		return Recycled;
	}
	return nullptr;
}
