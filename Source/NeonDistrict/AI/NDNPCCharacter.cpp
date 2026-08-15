// Copyright Neon District Sandbox. Public benchmark repo — original content only.

#include "AI/NDNPCCharacter.h"
#include "AI/NDNPCAIController.h"
#include "Systems/NDMissionSystem.h"
#include "Player/NDPlayerController.h"
#include "UI/NDHUDWidget.h"

#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

ANDNPCCharacter::ANDNPCCharacter()
{
	NPCVisual = GetMesh(); // mesh assigned in editor (Manny/Quinn/Mixamo import)
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

bool ANDNPCCharacter::Interact_Implementation(APlayerController* Instigator)
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
