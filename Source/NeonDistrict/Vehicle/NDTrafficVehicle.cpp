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
	// Assign a visible engine-shape mesh with zero asset dependency.
	if (UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")))
	{
		Mesh->SetStaticMesh(Cube);
		Mesh->SetRelativeScale3D(FVector(2.0f, 1.0f, 0.5f)); // car silhouette
	}
	// Tint traffic vehicles with a distinct neon color.
	if (UMaterialInterface* EngineMat = LoadObject<UMaterialInterface>(
		nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial")))
	{
		UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(EngineMat, this);
		if (MID)
		{
			MID->SetVectorParameterValue(TEXT("NeonColor"), FLinearColor(1.0f, 0.4f, 0.2f)); // orange
			MID->SetScalarParameterValue(TEXT("EmissiveStrength"), 3.0f);
			Mesh->SetMaterial(0, MID);
		}
	}
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
