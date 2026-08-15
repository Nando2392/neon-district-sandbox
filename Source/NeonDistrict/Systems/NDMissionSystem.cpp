// Copyright Neon District Sandbox. Public benchmark repo — original content only.

#include "Systems/NDMissionSystem.h"

void NDMissionSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	MissionStage = 0;
	ObjectiveText = FText::FromString(TEXT("Explora Neon District. Habla con Mei para un trabajo."));
}

void NDMissionSystem::AcceptMission(const FText& Objective, AActor* Target)
{
	MissionStage = 1;
	ObjectiveText = Objective;
	CurrentTarget = Target;
	OnMissionStageChanged.Broadcast(MissionStage);
}

void NDMissionSystem::AdvanceMission(const FText& NewObjective, AActor* NewTarget)
{
	if (MissionStage > 0 && MissionStage < 3)
	{
		++MissionStage;
	}
	ObjectiveText = NewObjective;
	CurrentTarget = NewTarget;
	OnMissionStageChanged.Broadcast(MissionStage);
}

void NDMissionSystem::CompleteMission()
{
	MissionStage = 3;
	ObjectiveText = FText::FromString(TEXT("Entrega completada. ¡Neon District respira más tranquilo!"));
	CurrentTarget = nullptr;
	OnMissionStageChanged.Broadcast(MissionStage);
}

bool NDMissionSystem::GetMarkerLocation(FVector& OutLocation) const
{
	if (CurrentTarget)
	{
		OutLocation = CurrentTarget->GetActorLocation();
		return true;
	}
	return false;
}
