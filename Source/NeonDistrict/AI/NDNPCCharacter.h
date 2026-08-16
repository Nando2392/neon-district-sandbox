// Copyright Neon District Sandbox. Public benchmark repo — original content only.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Player/NDInteractable.h"
#include "NDNPCCharacter.generated.h"

class UNDHUDWidget;

UENUM(BlueprintType)
enum class ENPCMissionRole : uint8
{
	None       UMETA(DisplayName = "None"),
	MissionGiver UMETA(DisplayName = "Mission Giver"),   // Mei: accepts the job
	Package    UMETA(DisplayName = "Package"),            // pickup
	Delivery   UMETA(DisplayName = "Delivery")            // Nova: completes the job
};

/**
 * Human NPC (civilian or police) with visible skeletal mesh + animation in the
 * editor-assigned ABP. Variants (outfit index) give visual variety without new classes.
 * Mission NPCs drive the short mission via the Mission system.
 */
UCLASS()
class NEONDISTRICT_API ANDNPCCharacter : public ACharacter, public INDIInteractable
{
	GENERATED_BODY()

public:
	ANDNPCCharacter();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "NPC")
	void ConfigureNPC(bool bInPolice, const FString& InDisplayName, ENPCMissionRole InRole, int32 OutfitVariant);

	UFUNCTION(BlueprintPure, Category = "NPC")
	bool IsPolice() const { return bPolice; }

	UFUNCTION(BlueprintPure, Category = "NPC")
	FString GetNPCName() const { return DisplayName; }

	UFUNCTION(BlueprintPure, Category = "NPC")
	ENPCMissionRole GetMissionRole() const { return MissionRole; }

	UFUNCTION(BlueprintPure, Category = "NPC|Combat")
	float GetHealth() const { return Health; }

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
		class AController* EventInstigator, AActor* DamageCauser) override;

	// INDIInteractable
	UFUNCTION(BlueprintNativeEvent, Category = "Interaction")
	FText GetInteractionPrompt() const;
	virtual FText GetInteractionPrompt_Implementation() const override;

	UFUNCTION(BlueprintNativeEvent, Category = "Interaction")
	bool Interact(APlayerController* PlayerController);
	virtual bool Interact_Implementation(APlayerController* PlayerController) override;

	/** Assigned by the spawner: police hold a post, mission NPCs patrol tight. */
	UPROPERTY(EditAnywhere, Category = "NPC")
	TArray<FVector> PatrolPoints;

	UPROPERTY(EditAnywhere, Category = "NPC|Visual")
	int32 OutfitVariant = 0;

protected:
	UPROPERTY(VisibleAnywhere, Category = "NPC|Visual")
	TObjectPtr<UStaticMeshComponent> NPCVisual = nullptr;
	UPROPERTY(VisibleAnywhere, Category = "NPC|Visual")
	TObjectPtr<UStaticMeshComponent> NPCHead = nullptr;
	UPROPERTY(VisibleAnywhere, Category = "NPC|Visual")
	TObjectPtr<UStaticMeshComponent> NPCHair = nullptr;
	UPROPERTY(VisibleAnywhere, Category = "NPC|Visual")
	TObjectPtr<UStaticMeshComponent> NPCLeftArm = nullptr;
	UPROPERTY(VisibleAnywhere, Category = "NPC|Visual")
	TObjectPtr<UStaticMeshComponent> NPCRightArm = nullptr;
	UPROPERTY(VisibleAnywhere, Category = "NPC|Visual")
	TObjectPtr<UStaticMeshComponent> NPCLeftHand = nullptr;
	UPROPERTY(VisibleAnywhere, Category = "NPC|Visual")
	TObjectPtr<UStaticMeshComponent> NPCRightHand = nullptr;
	UPROPERTY(VisibleAnywhere, Category = "NPC|Visual")
	TObjectPtr<UStaticMeshComponent> NPCLeftLeg = nullptr;
	UPROPERTY(VisibleAnywhere, Category = "NPC|Visual")
	TObjectPtr<UStaticMeshComponent> NPCRightLeg = nullptr;
	UPROPERTY(VisibleAnywhere, Category = "NPC|Visual")
	TObjectPtr<UStaticMeshComponent> NPCLeftFoot = nullptr;
	UPROPERTY(VisibleAnywhere, Category = "NPC|Visual")
	TObjectPtr<UStaticMeshComponent> NPCRightFoot = nullptr;
	UPROPERTY(VisibleAnywhere, Category = "NPC|Visual")
	TObjectPtr<UStaticMeshComponent> NPCAccessory = nullptr;

private:
	void NotifyHUD(const FText& Message);
	AActor* FindMissionDelivery();

	bool bPolice = false;
	FString DisplayName = TEXT("Ciudadano");
	ENPCMissionRole MissionRole = ENPCMissionRole::None;
	int32 DialogueLine = 0;
	float Health = 100.0f;
};
