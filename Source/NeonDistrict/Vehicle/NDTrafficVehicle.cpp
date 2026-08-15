// Copyright Neon District Sandbox. Public benchmark repo — original content only.

#include "Vehicle/NDTrafficVehicle.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SplineComponent.h"
#include "Engine/World.h"

ANDTrafficVehicle::ANDTrafficVehicle()
{
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
}

void ANDTrafficVehicle::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!Route)
	{
		return;
	}

	const float SplineLength = Route->GetSplineLength();
	if (SplineLength <= 0.0f)
	{
		return;
	}

	DistanceAlongSpline += (bReverseDirection ? -SpeedCmPerSec : SpeedCmPerSec) * DeltaSeconds;
	if (DistanceAlongSpline > SplineLength)
	{
		DistanceAlongSpline -= SplineLength;
	}
	if (DistanceAlongSpline < 0.0f)
	{
		DistanceAlongSpline += SplineLength;
	}

	const FVector NewLocation = Route->GetLocationAtDistanceAlongSpline(DistanceAlongSpline, ESplineCoordinateSpace::World);
	const FRotator NewRotation = Route->GetRotationAtDistanceAlongSpline(DistanceAlongSpline, ESplineCoordinateSpace::World);

	// Kinematic placement: no physics integration, no spawn churn.
	SetActorLocationAndRotation(NewLocation, NewRotation);
}
