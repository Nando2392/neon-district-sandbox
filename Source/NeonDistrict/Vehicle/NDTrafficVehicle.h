// Copyright Neon District Sandbox. Public benchmark repo — original content only.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NDTrafficVehicle.generated.h"

class UStaticMeshComponent;
class USplineComponent;

/**
 * Ambient traffic: kinematic vehicle following a spline route at a constant
 * speed, looping. No physics, no per-frame actor spawning — cheap by design.
 */
UCLASS()
class NEONDISTRICT_API ANDTrafficVehicle : public AActor
{
	GENERATED_BODY()

public:
	ANDTrafficVehicle();

	virtual void Tick(float DeltaSeconds) override;

	/** Assign the route spline (set by the spawner). */
	void SetRoute(USplineComponent* InRoute) { Route = InRoute; }

	UPROPERTY(EditDefaultsOnly, Category = "Traffic")
	float SpeedCmPerSec = 400.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Traffic")
	bool bReverseDirection = false;

private:
	UPROPERTY(VisibleAnywhere, Category = "Traffic")
	TObjectPtr<UStaticMeshComponent> Mesh = nullptr;
	UPROPERTY(VisibleAnywhere, Category = "Traffic")
	TObjectPtr<UStaticMeshComponent> CabinMesh = nullptr;
	UPROPERTY(VisibleAnywhere, Category = "Traffic")
	TObjectPtr<UStaticMeshComponent> LightMesh = nullptr;
	UPROPERTY(VisibleAnywhere, Category = "Traffic")
	TObjectPtr<UStaticMeshComponent> WheelFL = nullptr;
	UPROPERTY(VisibleAnywhere, Category = "Traffic")
	TObjectPtr<UStaticMeshComponent> WheelFR = nullptr;
	UPROPERTY(VisibleAnywhere, Category = "Traffic")
	TObjectPtr<UStaticMeshComponent> WheelRL = nullptr;
	UPROPERTY(VisibleAnywhere, Category = "Traffic")
	TObjectPtr<UStaticMeshComponent> WheelRR = nullptr;

	UPROPERTY()
	TObjectPtr<USplineComponent> Route = nullptr;

	float DistanceAlongSpline = 0.0f;
};
