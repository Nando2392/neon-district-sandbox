// Copyright Neon District Sandbox. Public benchmark repo — original content only.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "NDMissionSystem.generated.h"

class AActor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNDOnMissionStageChanged, int32, NewStage);

/**
 * Short mission: "Entrega el paquete a Nova" — 4 stages:
 *   0 idle -> 1 accepted (objective set) -> 2 package picked up -> 3 delivered (completed).
 * Stage changes are event-driven so HUD and save stay in sync.
 */
UCLASS()
class NEONDISTRICT_API NDMissionSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintPure, Category = "Mission")
	int32 GetMissionStage() const { return MissionStage; }

	UFUNCTION(BlueprintPure, Category = "Mission")
	FText GetObjectiveText() const { return ObjectiveText; }

	UFUNCTION(BlueprintCallable, Category = "Mission")
	void AcceptMission(const FText& Objective, AActor* Target);

	UFUNCTION(BlueprintCallable, Category = "Mission")
	void AdvanceMission(const FText& NewObjective, AActor* NewTarget);

	UFUNCTION(BlueprintCallable, Category = "Mission")
	void CompleteMission();

	/** World-space location the HUD marker should point at (falls back to last target). */
	UFUNCTION(BlueprintPure, Category = "Mission")
	bool GetMarkerLocation(FVector& OutLocation) const;

	UPROPERTY(BlueprintAssignable, Category = "Mission")
	FNDOnMissionStageChanged OnMissionStageChanged;

private:
	int32 MissionStage = 0;
	FText ObjectiveText = FText::GetEmpty();

	UPROPERTY()
	TObjectPtr<AActor> CurrentTarget = nullptr;
};
