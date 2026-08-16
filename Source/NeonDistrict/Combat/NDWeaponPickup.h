// Copyright Neon District Sandbox. Public benchmark repo — original content only.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Player/NDInteractable.h"
#include "NDWeaponPickup.generated.h"

class UStaticMeshComponent;

/** Street pickup for the procedural blaster. Interacting equips the player. */
UCLASS()
class NEONDISTRICT_API ANDWeaponPickup : public AActor, public INDIInteractable
{
	GENERATED_BODY()

public:
	ANDWeaponPickup();

	virtual FText GetInteractionPrompt_Implementation() const override;
	virtual bool Interact_Implementation(APlayerController* PlayerController) override;

private:
	void Tint(UStaticMeshComponent* Mesh, const FLinearColor& Color, float Emissive);

	UPROPERTY(VisibleAnywhere, Category = "Weapon")
	TObjectPtr<USceneComponent> Root = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "Weapon")
	TObjectPtr<UStaticMeshComponent> Body = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "Weapon")
	TObjectPtr<UStaticMeshComponent> Barrel = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "Weapon")
	TObjectPtr<UStaticMeshComponent> Grip = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "Weapon")
	TObjectPtr<UStaticMeshComponent> GlowCore = nullptr;
};
