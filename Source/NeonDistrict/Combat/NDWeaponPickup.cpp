// Copyright Neon District Sandbox. Public benchmark repo — original content only.

#include "Combat/NDWeaponPickup.h"
#include "Player/NDPlayerController.h"
#include "Core/NDGameInstance.h"
#include "Audio/NDAudioManager.h"

#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

namespace
{
	const TCHAR* WeaponPickup_CubeMeshPath = TEXT("/Engine/BasicShapes/Cube.Cube");
	const TCHAR* WeaponPickup_SphereMeshPath = TEXT("/Engine/BasicShapes/Sphere.Sphere");
	const TCHAR* WeaponPickup_BasicShapeMatPath = TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial");
}

ANDWeaponPickup::ANDWeaponPickup()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	Body->SetupAttachment(Root);
	Body->SetRelativeScale3D(FVector(0.62f, 0.18f, 0.18f));
	Body->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	Body->SetGenerateOverlapEvents(true);

	Barrel = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Barrel"));
	Barrel->SetupAttachment(Root);
	Barrel->SetRelativeLocation(FVector(54.0f, 0.0f, 4.0f));
	Barrel->SetRelativeScale3D(FVector(0.38f, 0.08f, 0.08f));
	Barrel->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Grip = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Grip"));
	Grip->SetupAttachment(Root);
	Grip->SetRelativeLocation(FVector(-18.0f, 0.0f, -22.0f));
	Grip->SetRelativeRotation(FRotator(0.0f, 0.0f, -18.0f));
	Grip->SetRelativeScale3D(FVector(0.16f, 0.10f, 0.32f));
	Grip->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	GlowCore = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GlowCore"));
	GlowCore->SetupAttachment(Root);
	GlowCore->SetRelativeLocation(FVector(16.0f, 0.0f, 8.0f));
	GlowCore->SetRelativeScale3D(FVector(0.14f));
	GlowCore->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, WeaponPickup_CubeMeshPath))
	{
		Body->SetStaticMesh(Cube);
		Barrel->SetStaticMesh(Cube);
		Grip->SetStaticMesh(Cube);
	}
	if (UStaticMesh* Sphere = LoadObject<UStaticMesh>(nullptr, WeaponPickup_SphereMeshPath))
	{
		GlowCore->SetStaticMesh(Sphere);
	}

	Tint(Body, FLinearColor(0.05f, 0.06f, 0.09f), 0.1f);
	Tint(Barrel, FLinearColor(0.02f, 0.75f, 1.0f), 2.2f);
	Tint(Grip, FLinearColor(0.012f, 0.012f, 0.018f), 0.0f);
	Tint(GlowCore, FLinearColor(0.10f, 0.95f, 1.0f), 6.0f);
}

void ANDWeaponPickup::Tint(UStaticMeshComponent* Mesh, const FLinearColor& Color, float Emissive)
{
	if (!Mesh)
	{
		return;
	}
	if (UMaterialInterface* Mat = LoadObject<UMaterialInterface>(nullptr, WeaponPickup_BasicShapeMatPath))
	{
		UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(Mat, this);
		MID->SetVectorParameterValue(TEXT("Color"), Color);
		MID->SetVectorParameterValue(TEXT("BaseColor"), Color);
		MID->SetVectorParameterValue(TEXT("NeonColor"), Color);
		MID->SetScalarParameterValue(TEXT("EmissiveStrength"), Emissive);
		Mesh->SetMaterial(0, MID);
	}
}

FText ANDWeaponPickup::GetInteractionPrompt_Implementation() const
{
	return FText::FromString(TEXT("Recoger blaster urbano (E)"));
}

bool ANDWeaponPickup::Interact_Implementation(APlayerController* PlayerController)
{
	ANDPlayerController* NDPC = Cast<ANDPlayerController>(PlayerController);
	if (!NDPC)
	{
		return false;
	}

	NDPC->EquipWeapon(24);
	if (UNDGameInstance* GI = GetGameInstance<UNDGameInstance>())
	{
		if (UNDAudioManager* Audio = GI->GetAudioManager())
		{
			Audio->PlayUI();
		}
	}
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	UE_LOG(LogTemp, Log, TEXT("NeonDistrict: weapon pickup equipped by player"));
	return true;
}
