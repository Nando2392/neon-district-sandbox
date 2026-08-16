// Copyright Neon District Sandbox. Public benchmark repo — original content only.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NDWeaponProjectile.generated.h"

class USphereComponent;
class UStaticMeshComponent;

/**
 * Physical blaster projectile: simulates collision, applies damage/impulse,
 * then self-destructs. Uses engine BasicShapes only so the slice packages with
 * zero external asset dependency.
 */
UCLASS()
class NEONDISTRICT_API ANDWeaponProjectile : public AActor
{
	GENERATED_BODY()

public:
	ANDWeaponProjectile();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	void Launch(const FVector& Direction, AActor* InInstigatorActor);

protected:
	UFUNCTION()
	void HandleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		FVector NormalImpulse, const FHitResult& Hit);

private:
	UPROPERTY(VisibleAnywhere, Category = "Weapon")
	TObjectPtr<USphereComponent> Collision = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "Weapon")
	TObjectPtr<UStaticMeshComponent> Visual = nullptr;

	UPROPERTY()
	TObjectPtr<AActor> InstigatorActor = nullptr;

	float LifeSeconds = 2.5f;
	float Damage = 34.0f;
	float ImpulseStrength = 65000.0f;
	float LaunchSpeed = 5200.0f;
};
