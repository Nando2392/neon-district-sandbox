// Copyright Neon District Sandbox. Public benchmark repo — original content only.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NDCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
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

protected:
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TObjectPtr<USpringArmComponent> SpringArm = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TObjectPtr<UCameraComponent> FollowCamera = nullptr;

private:
	void AccumulateFootstepDistance(float DeltaSeconds);
	void PlayFootstep();

	float WalkedDistance = 0.0f;
	bool bSprinting = false;

	// Footstep cadence: interval at walk, scaled up at sprint.
	static constexpr float FootstepIntervalCm = 190.0f;
};
