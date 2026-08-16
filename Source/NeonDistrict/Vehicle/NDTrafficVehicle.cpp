// Copyright Neon District Sandbox. Public benchmark repo — original content only.

#include "Vehicle/NDTrafficVehicle.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SplineComponent.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"

namespace
{
	const TCHAR* TrafficCubePath = TEXT("/Engine/BasicShapes/Cube.Cube");
	const TCHAR* TrafficCylinderPath = TEXT("/Engine/BasicShapes/Cylinder.Cylinder");
	const TCHAR* TrafficMaterialPath = TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial");

	void TintTrafficPart(UStaticMeshComponent* Part, UObject* Owner, const FLinearColor& Color, float Emissive = 0.0f)
	{
		if (!Part)
		{
			return;
		}
		if (UMaterialInterface* Base = LoadObject<UMaterialInterface>(nullptr, TrafficMaterialPath))
		{
			UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(Base, Owner);
			MID->SetVectorParameterValue(TEXT("Color"), Color);
			MID->SetVectorParameterValue(TEXT("BaseColor"), Color);
			MID->SetScalarParameterValue(TEXT("Roughness"), Emissive > 0.0f ? 0.25f : 0.62f);
			MID->SetScalarParameterValue(TEXT("EmissiveStrength"), Emissive);
			Part->SetMaterial(0, MID);
		}
	}
}

ANDTrafficVehicle::ANDTrafficVehicle()
{
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
	// Traffic remains kinematic for density/performance, but its visual is a
	// proper compact sedan rather than a moving box.
	if (UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TrafficCubePath))
	{
		Mesh->SetStaticMesh(Cube);
		Mesh->SetRelativeScale3D(FVector(2.30f, 1.12f, 0.42f));
	}
	Mesh->SetCastShadow(true);
	Mesh->bCastDynamicShadow = true;

	auto AddTrafficPart = [&](const TCHAR* Name, const FVector& Location, const FVector& Scale)
	{
		UStaticMeshComponent* Part = CreateDefaultSubobject<UStaticMeshComponent>(Name);
		Part->SetupAttachment(Mesh);
		Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Part->SetRelativeLocation(Location);
		Part->SetRelativeScale3D(Scale);
		Part->SetCastShadow(true);
		Part->bCastDynamicShadow = true;
		return Part;
	};
	CabinMesh = AddTrafficPart(TEXT("CabinMesh"), FVector(-16.0f, 0.0f, 48.0f), FVector(0.96f, 0.72f, 0.30f));
	LightMesh = AddTrafficPart(TEXT("LightMesh"), FVector(116.0f, 0.0f, 14.0f), FVector(0.07f, 0.88f, 0.08f));
	WheelFL = AddTrafficPart(TEXT("WheelFL"), FVector(88.0f, 68.0f, -22.0f), FVector(0.28f, 0.28f, 0.19f));
	WheelFR = AddTrafficPart(TEXT("WheelFR"), FVector(88.0f, -68.0f, -22.0f), FVector(0.28f, 0.28f, 0.19f));
	WheelRL = AddTrafficPart(TEXT("WheelRL"), FVector(-94.0f, 68.0f, -22.0f), FVector(0.28f, 0.28f, 0.19f));
	WheelRR = AddTrafficPart(TEXT("WheelRR"), FVector(-94.0f, -68.0f, -22.0f), FVector(0.28f, 0.28f, 0.19f));
	if (UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TrafficCubePath))
	{
		CabinMesh->SetStaticMesh(Cube);
		LightMesh->SetStaticMesh(Cube);
	}
	if (UStaticMesh* Cylinder = LoadObject<UStaticMesh>(nullptr, TrafficCylinderPath))
	{
		WheelFL->SetStaticMesh(Cylinder);
		WheelFR->SetStaticMesh(Cylinder);
		WheelRL->SetStaticMesh(Cylinder);
		WheelRR->SetStaticMesh(Cylinder);
		WheelFL->SetRelativeRotation(FRotator(0.0f, 0.0f, 90.0f));
		WheelFR->SetRelativeRotation(FRotator(0.0f, 0.0f, 90.0f));
		WheelRL->SetRelativeRotation(FRotator(0.0f, 0.0f, 90.0f));
		WheelRR->SetRelativeRotation(FRotator(0.0f, 0.0f, 90.0f));
	}
	TintTrafficPart(Mesh, this, FLinearColor(0.80f, 0.14f, 0.045f));
	TintTrafficPart(CabinMesh, this, FLinearColor(0.012f, 0.060f, 0.110f));
	TintTrafficPart(LightMesh, this, FLinearColor(1.00f, 0.74f, 0.28f), 2.5f);
	TintTrafficPart(WheelFL, this, FLinearColor(0.006f, 0.007f, 0.010f));
	TintTrafficPart(WheelFR, this, FLinearColor(0.006f, 0.007f, 0.010f));
	TintTrafficPart(WheelRL, this, FLinearColor(0.006f, 0.007f, 0.010f));
	TintTrafficPart(WheelRR, this, FLinearColor(0.006f, 0.007f, 0.010f));
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
