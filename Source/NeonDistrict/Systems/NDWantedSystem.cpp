// Copyright Neon District Sandbox. Public benchmark repo — original content only.

#include "Systems/NDWantedSystem.h"
#include "Core/NDPerfConstants.h"

void UNDWantedSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	WantedLevel = 0;
	HeatAccumulator = 0.0f;
}

void UNDWantedSystem::ReportDetection(float Intensity)
{
	bEvading = false;
	HeatAccumulator = FMath::Min(HeatAccumulator + HeatPerDetection * Intensity, Level3Threshold + 1.0f);
	OnHeatEvent.Broadcast(HeatAccumulator / Level3Threshold);

	const int32 LevelBefore = WantedLevel;
	if (HeatAccumulator >= Level3Threshold)      ApplyLevel(3);
	else if (HeatAccumulator >= Level2Threshold) ApplyLevel(2);
	else if (HeatAccumulator >= Level1Threshold) ApplyLevel(1);

	if (WantedLevel != LevelBefore)
	{
		// Restart the decay timer with a fresh delay on level change.
		GetWorld()->GetTimerManager().ClearTimer(DecayTimerHandle);
		GetWorld()->GetTimerManager().SetTimer(DecayTimerHandle, this, &UNDWantedSystem::TickDecay,
			NDPerf::WantedDecayInterval, true);
	}
}

void UNDWantedSystem::ReportEvasion()
{
	bEvading = true;
}

void UNDWantedSystem::SetWantedLevel(int32 NewLevel)
{
	ApplyLevel(FMath::Clamp(NewLevel, 0, NDPerf::MaxWantedLevel));
}

void UNDWantedSystem::ClearWanted()
{
	HeatAccumulator = 0.0f;
	ApplyLevel(0);
	GetWorld()->GetTimerManager().ClearTimer(DecayTimerHandle);
}

void UNDWantedSystem::TickDecay(float /*DeltaSeconds*/)
{
	if (!bEvading)
	{
		return; // still detected: heat only decays after the evasion window
	}
	HeatAccumulator = FMath::Max(0.0f, HeatAccumulator - (Level3Threshold / 3.0f));
	const int32 LevelBefore = WantedLevel;
	if (HeatAccumulator < Level1Threshold)      ApplyLevel(0);
	else if (HeatAccumulator < Level2Threshold) ApplyLevel(1);
	else if (HeatAccumulator < Level3Threshold) ApplyLevel(2);

	if (WantedLevel == 0 && LevelBefore > 0)
	{
		GetWorld()->GetTimerManager().ClearTimer(DecayTimerHandle);
	}
}

void UNDWantedSystem::ApplyLevel(int32 NewLevel)
{
	if (WantedLevel == NewLevel)
	{
		return;
	}
	WantedLevel = NewLevel;
	OnWantedLevelChanged.Broadcast(WantedLevel);
	OnHeatEvent.Broadcast(WantedLevel > 0 ? (HeatAccumulator / Level3Threshold) : 0.0f);
}
