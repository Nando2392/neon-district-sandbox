// Copyright Neon District Sandbox. Public benchmark repo — original content only.

#include "AI/NDNPCCharacter.h"
#include "AI/NDNPCAIController.h"
#include "Systems/NDMissionSystem.h"
#include "Player/NDPlayerController.h"
#include "UI/NDHUDWidget.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/SkeletalMesh.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	const TCHAR* CubeMeshPath = TEXT("/Engine/BasicShapes/Cube.Cube");
	const TCHAR* SphereMeshPath = TEXT("/Engine/BasicShapes/Sphere.Sphere");
	const TCHAR* BasicShapeMatPath = TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial");
	// Engine-cooked humanoid mannequin (head, torso, limbs, face) + walk anims.
	const TCHAR* MannequinMeshPath = TEXT("/Engine/Tutorial/SubEditors/TutorialAssets/Character/TutorialTPP.TutorialTPP");
	const TCHAR* MannequinAnimBPPath = TEXT("/Engine/Tutorial/SubEditors/TutorialAssets/Character/TutorialTPP_AnimBlueprint.TutorialTPP_AnimBlueprint_C");
	const TCHAR* MannequinMatPath = TEXT("/Engine/Tutorial/SubEditors/TutorialAssets/Character/TutorialTPP_Mat.TutorialTPP_Mat");

	void TintPart(UStaticMeshComponent* Mesh, UObject* Owner, const FLinearColor& Color, float Emissive = 0.0f)
	{
		if (!Mesh)
		{
			return;
		}
		if (UMaterialInterface* EngineMat = LoadObject<UMaterialInterface>(nullptr, BasicShapeMatPath))
		{
			UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(EngineMat, Owner);
			MID->SetVectorParameterValue(TEXT("Color"), Color);
			MID->SetVectorParameterValue(TEXT("BaseColor"), Color);
			MID->SetVectorParameterValue(TEXT("NeonColor"), Color);
			MID->SetScalarParameterValue(TEXT("EmissiveStrength"), Emissive);
			Mesh->SetMaterial(0, MID);
		}
	}
}

ANDNPCCharacter::ANDNPCCharacter()
{
	// Human NPCs: try the engine mannequin (real humanoid with face + walk
	// anims). If it cannot load, fall back to segmented primitives so the
	// NPC still reads as a human-ish figure in packaged builds.
	USkeletalMeshComponent* Skel = GetMesh();
	bool bMannequinLoaded = false;
	if (Skel)
	{
		Skel->SetGenerateOverlapEvents(false);
		Skel->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		if (USkeletalMesh* Mannequin = LoadObject<USkeletalMesh>(nullptr, MannequinMeshPath))
		{
			Skel->SetSkeletalMesh(Mannequin);
			Skel->SetVisibility(true);
			Skel->SetRelativeLocation(FVector(0, 0, -88.0f));
			Skel->SetRelativeScale3D(FVector(0.96f, 0.96f, 0.96f));
			if (UClass* ABP = LoadObject<UClass>(nullptr, MannequinAnimBPPath))
			{
				Skel->SetAnimInstanceClass(ABP);
			}
			bMannequinLoaded = true;
		}
		else
		{
			Skel->SetVisibility(false);
		}
	}

	// Segmented human-like body (fallback when the mannequin is unavailable).
	NPCVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("NPCVisual"));
	NPCVisual->SetupAttachment(RootComponent);
	NPCVisual->SetRelativeScale3D(FVector(0.54f, 0.34f, 0.62f));
	NPCVisual->SetRelativeLocation(FVector(0, 0, 16.0f));
	NPCVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	NPCVisual->bCastDynamicShadow = true;

	NPCHead = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("NPCHead"));
	NPCHead->SetupAttachment(RootComponent);
	NPCHead->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	NPCHead->SetRelativeLocation(FVector(0, 0, 104.0f));
	NPCHead->SetRelativeScale3D(FVector(0.38f, 0.34f, 0.40f));

	NPCHair = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("NPCHair"));
	NPCHair->SetupAttachment(RootComponent);
	NPCHair->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	NPCHair->SetRelativeLocation(FVector(0, -4.0f, 128.0f));
	NPCHair->SetRelativeScale3D(FVector(0.39f, 0.34f, 0.12f));

	NPCLeftArm = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("NPCLeftArm"));
	NPCLeftArm->SetupAttachment(RootComponent);
	NPCLeftArm->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	NPCLeftArm->SetRelativeLocation(FVector(0, -46.0f, 22.0f));
	NPCLeftArm->SetRelativeScale3D(FVector(0.18f, 0.17f, 0.50f));

	NPCRightArm = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("NPCRightArm"));
	NPCRightArm->SetupAttachment(RootComponent);
	NPCRightArm->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	NPCRightArm->SetRelativeLocation(FVector(0, 46.0f, 22.0f));
	NPCRightArm->SetRelativeScale3D(FVector(0.18f, 0.17f, 0.50f));

	NPCLeftHand = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("NPCLeftHand"));
	NPCLeftHand->SetupAttachment(RootComponent);
	NPCLeftHand->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	NPCLeftHand->SetRelativeLocation(FVector(0, -46.0f, -18.0f));
	NPCLeftHand->SetRelativeScale3D(FVector(0.16f, 0.15f, 0.16f));

	NPCRightHand = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("NPCRightHand"));
	NPCRightHand->SetupAttachment(RootComponent);
	NPCRightHand->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	NPCRightHand->SetRelativeLocation(FVector(0, 46.0f, -18.0f));
	NPCRightHand->SetRelativeScale3D(FVector(0.16f, 0.15f, 0.16f));

	NPCLeftLeg = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("NPCLeftLeg"));
	NPCLeftLeg->SetupAttachment(RootComponent);
	NPCLeftLeg->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	NPCLeftLeg->SetRelativeLocation(FVector(0, -17.0f, -58.0f));
	NPCLeftLeg->SetRelativeScale3D(FVector(0.20f, 0.16f, 0.64f));

	NPCRightLeg = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("NPCRightLeg"));
	NPCRightLeg->SetupAttachment(RootComponent);
	NPCRightLeg->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	NPCRightLeg->SetRelativeLocation(FVector(0, 17.0f, -58.0f));
	NPCRightLeg->SetRelativeScale3D(FVector(0.20f, 0.16f, 0.64f));

	NPCLeftFoot = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("NPCLeftFoot"));
	NPCLeftFoot->SetupAttachment(RootComponent);
	NPCLeftFoot->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	NPCLeftFoot->SetRelativeLocation(FVector(18.0f, -17.0f, -124.0f));
	NPCLeftFoot->SetRelativeScale3D(FVector(0.28f, 0.18f, 0.10f));

	NPCRightFoot = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("NPCRightFoot"));
	NPCRightFoot->SetupAttachment(RootComponent);
	NPCRightFoot->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	NPCRightFoot->SetRelativeLocation(FVector(18.0f, 17.0f, -124.0f));
	NPCRightFoot->SetRelativeScale3D(FVector(0.28f, 0.18f, 0.10f));

	NPCAccessory = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("NPCAccessory"));
	NPCAccessory->SetupAttachment(RootComponent);
	NPCAccessory->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	NPCAccessory->SetRelativeLocation(FVector(2.0f, -24.0f, 42.0f));
	NPCAccessory->SetRelativeScale3D(FVector(0.36f, 0.06f, 0.30f));

	if (UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, CubeMeshPath))
	{
		NPCVisual->SetStaticMesh(Cube);
		NPCHair->SetStaticMesh(Cube);
		NPCLeftArm->SetStaticMesh(Cube);
		NPCRightArm->SetStaticMesh(Cube);
		NPCLeftLeg->SetStaticMesh(Cube);
		NPCRightLeg->SetStaticMesh(Cube);
		NPCLeftFoot->SetStaticMesh(Cube);
		NPCRightFoot->SetStaticMesh(Cube);
		NPCAccessory->SetStaticMesh(Cube);
	}
	if (UStaticMesh* Sphere = LoadObject<UStaticMesh>(nullptr, SphereMeshPath))
	{
		NPCHead->SetStaticMesh(Sphere);
		NPCLeftHand->SetStaticMesh(Sphere);
		NPCRightHand->SetStaticMesh(Sphere);
	}

	// Hide the primitive proxy when the mannequin is showing.
	const bool bShowProxy = !bMannequinLoaded;
	NPCVisual->SetVisibility(bShowProxy);
	NPCHead->SetVisibility(bShowProxy);
	NPCHair->SetVisibility(bShowProxy);
	NPCLeftArm->SetVisibility(bShowProxy);
	NPCRightArm->SetVisibility(bShowProxy);
	NPCLeftHand->SetVisibility(bShowProxy);
	NPCRightHand->SetVisibility(bShowProxy);
	NPCLeftLeg->SetVisibility(bShowProxy);
	NPCRightLeg->SetVisibility(bShowProxy);
	NPCLeftFoot->SetVisibility(bShowProxy);
	NPCRightFoot->SetVisibility(bShowProxy);
	NPCAccessory->SetVisibility(bShowProxy);
}

void ANDNPCCharacter::BeginPlay()
{
	Super::BeginPlay();
	// Ensure a controller exists so the FSM runs.
	SpawnDefaultController();
}

float ANDNPCCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	const float Applied = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	Health = FMath::Max(0.0f, Health - DamageAmount);
	TintPart(NPCAccessory, this, FLinearColor(1.0f, 0.10f, 0.08f), 3.5f);
	UE_LOG(LogTemp, Log, TEXT("NeonDistrict: NPC %s took %.1f damage, health=%.1f"), *GetName(), DamageAmount, Health);
	return Applied > 0.0f ? Applied : DamageAmount;
}

void ANDNPCCharacter::ConfigureNPC(bool bInPolice, const FString& InDisplayName, ENPCMissionRole InRole, int32 OutfitVariantIn)
{
	bPolice = bInPolice;
	DisplayName = InDisplayName;
	MissionRole = InRole;
	OutfitVariant = OutfitVariantIn;

	// Role color (neon palette to match the district).
	FLinearColor RoleColor;
	FLinearColor AccessoryColor;
	if (MissionRole == ENPCMissionRole::MissionGiver) // Mei
	{ RoleColor = FLinearColor(1.0f, 0.10f, 0.60f); AccessoryColor = FLinearColor(0.12f, 0.9f, 1.0f); }
	else if (MissionRole == ENPCMissionRole::Delivery) // Nova
	{ RoleColor = FLinearColor(1.0f, 0.90f, 0.10f); AccessoryColor = FLinearColor(0.65f, 0.18f, 1.0f); }
	else if (MissionRole == ENPCMissionRole::Package)
	{ RoleColor = FLinearColor(1.0f, 0.60f, 0.90f); AccessoryColor = FLinearColor(0.75f, 0.45f, 0.18f); }
	else if (bPolice)
	{ RoleColor = FLinearColor(0.02f, 0.08f, 0.18f); AccessoryColor = FLinearColor(0.10f, 0.90f, 1.00f); }
	else
	{ RoleColor = FLinearColor(0.10f + 0.06f * (OutfitVariant % 4), 0.28f + 0.13f * (OutfitVariant % 3), 0.16f + 0.08f * (OutfitVariant % 5)); AccessoryColor = FLinearColor(0.20f, 1.0f, 0.40f); }

	TintPart(NPCVisual, this, RoleColor, (MissionRole != ENPCMissionRole::None || bPolice) ? 1.2f : 0.0f);
	TintPart(NPCHead, this, FLinearColor(0.78f, 0.52f, 0.36f), 0.0f);
	TintPart(NPCHair, this, bPolice ? FLinearColor(0.02f, 0.02f, 0.04f) : FLinearColor(0.05f, 0.025f, 0.015f), 0.0f);
	TintPart(NPCLeftArm, this, RoleColor * 0.75f, 0.0f);
	TintPart(NPCRightArm, this, RoleColor * 0.75f, 0.0f);
	TintPart(NPCLeftHand, this, FLinearColor(0.78f, 0.52f, 0.36f), 0.0f);
	TintPart(NPCRightHand, this, FLinearColor(0.78f, 0.52f, 0.36f), 0.0f);
	TintPart(NPCLeftLeg, this, FLinearColor(0.025f, 0.03f, 0.055f), 0.0f);
	TintPart(NPCRightLeg, this, FLinearColor(0.025f, 0.03f, 0.055f), 0.0f);
	TintPart(NPCLeftFoot, this, FLinearColor(0.005f, 0.006f, 0.01f), 0.0f);
	TintPart(NPCRightFoot, this, FLinearColor(0.005f, 0.006f, 0.01f), 0.0f);
	TintPart(NPCAccessory, this, AccessoryColor, (MissionRole != ENPCMissionRole::None || bPolice) ? 3.5f : 0.7f);

	// Tint the engine mannequin by role so Mei/Nova/police/civilians stay
	// distinguishable in screenshots (outfit color + police cap accent).
	if (USkeletalMeshComponent* Skel = GetMesh())
	{
		if (UMaterialInterface* Mat = LoadObject<UMaterialInterface>(nullptr, MannequinMatPath))
		{
			UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(Mat, this);
			MID->SetVectorParameterValue(TEXT("Color"), RoleColor);
			Skel->SetMaterial(0, MID);
		}
	}
	if (bPolice)
	{
		NPCAccessory->SetRelativeLocation(FVector(0.0f, 0.0f, 132.0f));
		NPCAccessory->SetRelativeScale3D(FVector(0.44f, 0.34f, 0.10f));
	}
	else if (MissionRole == ENPCMissionRole::Delivery)
	{
		NPCAccessory->SetRelativeLocation(FVector(2.0f, 24.0f, 44.0f));
		NPCAccessory->SetRelativeScale3D(FVector(0.36f, 0.06f, 0.32f));
	}

	if (ANDNPCAIController* AIC = Cast<ANDNPCAIController>(GetController()))
	{
		AIC->SetPatrolPoints(PatrolPoints, bPolice);
	}
}

FText ANDNPCCharacter::GetInteractionPrompt_Implementation() const
{
	switch (MissionRole)
	{
	case ENPCMissionRole::MissionGiver:
		return FText::Format(FText::FromString(TEXT("Hablar con {0} (E)")), FText::FromString(DisplayName));
	case ENPCMissionRole::Package:
		return FText::FromString(TEXT("Recoger paquete (E)"));
	case ENPCMissionRole::Delivery:
		return FText::FromString(TEXT("Entregar a Nova (E)"));
	default:
		return FText::Format(FText::FromString(TEXT("Saludar a {0} (E)")), FText::FromString(DisplayName));
	}
}

bool ANDNPCCharacter::Interact_Implementation(APlayerController* PlayerController)
{
	UNDMissionSystem* Mission = GetGameInstance()->GetSubsystem<UNDMissionSystem>();
	if (!Mission)
	{
		return false;
	}

	const int32 Stage = Mission->GetMissionStage();

	switch (MissionRole)
	{
	case ENPCMissionRole::MissionGiver:
		if (Stage == 0)
		{
			Mission->AcceptMission(
				FText::FromString(TEXT("Mei: lleva este paquete a Nova en la avenida norte. No te lo pierdas.")),
				this);
			NotifyHUD(FText::FromString(TEXT("Misión aceptada: entrega el paquete a Nova (marcador en el HUD).")));
		}
		else
		{
			NotifyHUD(FText::FromString(TEXT("Mei: el paquete sigue esperando. Llévaselo a Nova.")));
		}
		return true;

	case ENPCMissionRole::Package:
		if (Stage == 1)
		{
			Mission->AdvanceMission(
				FText::FromString(TEXT("Paquete recogido. Entrégalo a Nova.")),
				FindMissionDelivery());
			NotifyHUD(FText::FromString(TEXT("Paquete recogido. Búscala en la avenida norte.")));
		}
		else
		{
			NotifyHUD(FText::FromString(TEXT("Aquí no hay nada para ti, chico.")));
		}
		return true;

	case ENPCMissionRole::Delivery:
		if (Stage == 2)
		{
			Mission->CompleteMission();
			NotifyHUD(FText::FromString(TEXT("Nova: ¡perfecto! El distrito te debe una. (+credibilidad)")));
		}
		else if (Stage < 2)
		{
			NotifyHUD(FText::FromString(TEXT("Nova: ¿qué quieres? Vuelve cuando tengas algo que entregar.")));
		}
		else
		{
			NotifyHUD(FText::FromString(TEXT("Nova: gran trabajo hoy. El neón brilla más contigo.")));
		}
		return true;

	default:
		// Generic small talk — short, varied, no branching system needed.
		++DialogueLine;
		if (DialogueLine % 3 == 0)
		{
			NotifyHUD(FText::FromString(TEXT("Ciudadano: los coches patrulla llevan toda la noche rondando...")));
		}
		else
		{
			NotifyHUD(FText::FromString(TEXT("Ciudadano: tranquilo aquí, mientras no metas las manos donde no debes.")));
		}
		return true;
	}
}

AActor* ANDNPCCharacter::FindMissionDelivery()
{
	TArray<AActor*> Found;
	UGameplayStatics::GetAllActorsOfClass(this, ANDNPCCharacter::StaticClass(), Found);
	for (AActor* Actor : Found)
	{
		ANDNPCCharacter* NPC = Cast<ANDNPCCharacter>(Actor);
		if (NPC && NPC->GetMissionRole() == ENPCMissionRole::Delivery)
		{
			return NPC;
		}
	}
	return nullptr;
}

void ANDNPCCharacter::NotifyHUD(const FText& Message)
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		if (ANDPlayerController* NDPC = Cast<ANDPlayerController>(PC))
		{
			if (UNDHUDWidget* HUD = NDPC->GetHUDWidget())
			{
				HUD->ShowNotification(Message);
			}
		}
	}
}
