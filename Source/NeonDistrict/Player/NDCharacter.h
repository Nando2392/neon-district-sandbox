// Copyright Neon District Sandbox. Public benchmark repo — original content only.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NDCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UStaticMeshComponent;
class UNDAudioManager;

/**
 * Playable third-person human. Spring-arm camera with collision, walk/run/sprint/jump,
 * footstep feedback by distance travelled (no per-frame audio spam).
 */
UCLASS()
class NEONDISTRICT_API ANDCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ANDCharacter();

	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	void SetSprinting(bool bSprinting);

	UFUNCTION(BlueprintPure, Category = "Character")
	float GetCurrentSpeed() const { return GetVelocity().Size2D(); }

	void SetWeaponVisible(bool bVisible);

protected:
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TObjectPtr<USpringArmComponent> SpringArm = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TObjectPtr<UCameraComponent> FollowCamera = nullptr;

	/** Visible body proxy (engine Cube + neon-tinted material). */
	UPROPERTY(VisibleAnywhere, Category = "Character")
	TObjectPtr<UStaticMeshComponent> PlayerBody = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "Character")
	TObjectPtr<UStaticMeshComponent> PlayerHead = nullptr;
	UPROPERTY(VisibleAnywhere, Category = "Character")
	TObjectPtr<UStaticMeshComponent> PlayerHair = nullptr;
	UPROPERTY(VisibleAnywhere, Category = "Character")
	TObjectPtr<UStaticMeshComponent> PlayerLeftArm = nullptr;
	UPROPERTY(VisibleAnywhere, Category = "Character")
	TObjectPtr<UStaticMeshComponent> PlayerRightArm = nullptr;
	UPROPERTY(VisibleAnywhere, Category = "Character")
	TObjectPtr<UStaticMeshComponent> PlayerLeftHand = nullptr;
	UPROPERTY(VisibleAnywhere, Category = "Character")
	TObjectPtr<UStaticMeshComponent> PlayerRightHand = nullptr;
	UPROPERTY(VisibleAnywhere, Category = "Character")
	TObjectPtr<UStaticMeshComponent> PlayerLeftLeg = nullptr;
	UPROPERTY(VisibleAnywhere, Category = "Character")
	TObjectPtr<UStaticMeshComponent> PlayerRightLeg = nullptr;
	UPROPERTY(VisibleAnywhere, Category = "Character")
	TObjectPtr<UStaticMeshComponent> PlayerLeftFoot = nullptr;
	UPROPERTY(VisibleAnywhere, Category = "Character")
	TObjectPtr<UStaticMeshComponent> PlayerRightFoot = nullptr;
	UPROPERTY(VisibleAnywhere, Category = "Character")
	TObjectPtr<UStaticMeshComponent> PlayerJacket = nullptr;
	UPROPERTY(VisibleAnywhere, Category = "Character|Weapon")
	TObjectPtr<UStaticMeshComponent> WeaponBody = nullptr;
	UPROPERTY(VisibleAnywhere, Category = "Character|Weapon")
	TObjectPtr<UStaticMeshComponent> WeaponBarrel = nullptr;
	UPROPERTY(VisibleAnywhere, Category = "Character|Weapon")
	TObjectPtr<UStaticMeshComponent> WeaponCore = nullptr;

private:
	void AccumulateFootstepDistance(float DeltaSeconds);
	void PlayFootstep();

	float WalkedDistance = 0.0f;
	bool bSprinting = false;

	// Footstep cadence: interval at walk, scaled up at sprint.
	static constexpr float FootstepIntervalCm = 190.0f;
};
