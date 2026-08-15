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
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

ANDNPCCharacter::ANDNPCCharacter()
{
	// Human NPCs need no physics but do need a simple visual proxy that works
	// with zero asset dependency. The engine SkeletalMesh is empty, so attach a
	// capsule + body mesh (Cube scaled to a standing figure). The mesh is tinted
	// per-role by ConfigureNPC.
	NPCVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("NPCVisual"));
	NPCVisual->SetupAttachment(RootComponent);
	if (UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")))
	{
		NPCVisual->SetStaticMesh(Cube);
		NPCVisual->SetRelativeScale3D(FVector(0.5f, 0.5f, 1.8f)); // ~humanoid silhouette
		NPCVisual->SetRelativeLocation(FVector(0, 0, -90.0f)); // align with capsule standing height
		NPCVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		NPCVisual->bCastDynamicShadow = false;
	}
}

void ANDNPCCharacter::BeginPlay()
{
	Super::BeginPlay();
	// Ensure a controller exists so the FSM runs.
	SpawnDefaultController();
}

void ANDNPCCharacter::ConfigureNPC(bool bInPolice, const FString& InDisplayName, ENPCMissionRole InRole, int32 OutfitVariantIn)
{
	bPolice = bInPolice;
	DisplayName = InDisplayName;
	MissionRole = InRole;
	OutfitVariant = OutfitVariantIn;

	// Tint the static mesh body per role using the engine neutral material.
	static FName ColorParam = TEXT("NeonColor");
	if (NPCVisual && NPCVisual->GetStaticMesh())
	{
		if (UMaterialInterface* BaseMat = NPCVisual->GetMaterial(0))
		{
			if (UMaterialInstanceDynamic* ExistingMID = Cast<UMaterialInstanceDynamic>(BaseMat))
			{
				NPCVisual->SetMaterial(0, ExistingMID);
			}
			else if (UMaterial* Parent = Cast<UMaterial>(BaseMat))
			{
				UMaterialInstanceDynamic* NewMID = UMaterialInstanceDynamic::Create(Parent, this);
				NPCVisual->SetMaterial(0, NewMID);
			}
			else
			{
				// Not a material instance we can tint — create one from BasicShapeMaterial.
				UMaterialInterface* EngineMat = LoadObject<UMaterialInterface>(
					nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
				if (EngineMat)
				{
					UMaterialInstanceDynamic* NewMID = UMaterialInstanceDynamic::Create(EngineMat, this);
					NPCVisual->SetMaterial(0, NewMID);
				}
			}
		}
	}

	// Role color (neon palette to match the district).
	FLinearColor RoleColor;
	if (MissionRole == ENPCMissionRole::MissionGiver) // Mei
		RoleColor = FLinearColor(1.0f, 0.10f, 0.60f);   // magenta
	else if (MissionRole == ENPCMissionRole::Delivery) // Nova
		RoleColor = FLinearColor(1.0f, 0.90f, 0.10f);   // gold
	else if (MissionRole == ENPCMissionRole::Package)
		RoleColor = FLinearColor(1.0f, 0.60f, 0.90f);   // pink
	else if (bPolice)
		RoleColor = FLinearColor(0.10f, 0.90f, 1.00f);  // cyan
	else
		RoleColor = FLinearColor(0.20f, 1.0f, 0.40f);   // green

	if (NPCVisual)
	{
		if (UMaterialInstanceDynamic* TintMID = Cast<UMaterialInstanceDynamic>(NPCVisual->GetMaterial(0)))
		{
			TintMID->SetVectorParameterValue(ColorParam, RoleColor);
			TintMID->SetScalarParameterValue(TEXT("EmissiveStrength"), 3.0f);
		}
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
