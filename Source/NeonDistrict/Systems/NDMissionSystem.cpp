// Copyright Neon District Sandbox. Public benchmark repo — original content only.

#include "Systems/NDMissionSystem.h"

void UNDMissionSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	MissionStage = 0;
	ObjectiveText = FText::FromString(TEXT("Explora Neon District. Habla con Mei para un trabajo."));
}

void UNDMissionSystem::AcceptMission(const FText& Objective, AActor* Target)
{
	MissionStage = 1;
	ObjectiveText = Objective;
	CurrentTarget = Target;
	OnMissionStageChanged.Broadcast(MissionStage);
}

void UNDMissionSystem::AdvanceMission(const FText& NewObjective, AActor* NewTarget)
{
	if (MissionStage > 0 && MissionStage < 3)
	{
		++MissionStage;
	}
	ObjectiveText = NewObjective;
	CurrentTarget = NewTarget;
	OnMissionStageChanged.Broadcast(MissionStage);
}

void UNDMissionSystem::CompleteMission()
{
	MissionStage = 3;
	ObjectiveText = FText::FromString(TEXT("Entrega completada. ¡Neon District respira más tranquilo!"));
	CurrentTarget = nullptr;
	OnMissionStageChanged.Broadcast(MissionStage);
}

bool UNDMissionSystem::GetMarkerLocation(FVector& OutLocation) const
{
	if (CurrentTarget)
	{
		OutLocation = CurrentTarget->GetActorLocation();
		return true;
	}
	return false;
}
