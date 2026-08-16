// Copyright Neon District Sandbox. Public benchmark repo — original content only.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Player/NDInteractable.h"
#include "NDVehicle.generated.h"

class UChaosWheeledVehicleMovementComponent;
class UStaticMeshComponent;
class USceneComponent;
class UCameraComponent;
class USpringArmComponent;
class UProceduralMeshComponent;
class APlayerController;

/**
 * Driveable Chaos vehicle. Enter/exit, throttle/brake/steer, driving camera,
 * engine audio hookup. Wheel setup is created in code when empty so the
 * vehicle is functional with zero asset dependency; fine-tuning happens in-editor.
 */
UCLASS()
class NEONDISTRICT_API ANDVehicle : public APawn, public INDIInteractable
{
	GENERATED_BODY()

public:
	ANDVehicle();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	// --- Driving input (fed by the player controller) ---
	void ApplyDriveInput(float ThrottleBrake, float Steering);
	void SetHandbrake(bool bEngaged);

	// --- Enter / exit ---
	void EnterVehicle(APlayerController* PC);
	void ExitVehicle(APlayerController* PC);

	FText GetDisplayName() const { return DisplayName; }

	// INDIInteractable
	UFUNCTION(BlueprintNativeEvent, Category = "Interaction")
	FText GetInteractionPrompt() const;
	virtual FText GetInteractionPrompt_Implementation() const override;

	UFUNCTION(BlueprintNativeEvent, Category = "Interaction")
	bool Interact(APlayerController* PlayerController);
	virtual bool Interact_Implementation(APlayerController* PlayerController) override;

	UFUNCTION(BlueprintPure, Category = "Vehicle")
	float GetForwardSpeedKmh() const;

	/** Collision feedback: FX + alert audio on hard impacts (gated, no per-frame spam). */
	virtual void NotifyHit(class UPrimitiveComponent* MyComp, AActor* Other, class UPrimitiveComponent* OtherComp,
		bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;

protected:
	UPROPERTY(VisibleAnywhere, Category = "Vehicle")
	TObjectPtr<USceneComponent> VehicleRoot = nullptr;
	UPROPERTY(VisibleAnywhere, Category = "Vehicle")
	TObjectPtr<UStaticMeshComponent> BodyMesh = nullptr;
	UPROPERTY(VisibleAnywhere, Category = "Vehicle")
	TObjectPtr<UStaticMeshComponent> AuthoredBodyMesh = nullptr;
	UPROPERTY(VisibleAnywhere, Category = "Vehicle")
	TObjectPtr<UProceduralMeshComponent> BodyVisualMesh = nullptr;
	UPROPERTY(VisibleAnywhere, Category = "Vehicle")
	TObjectPtr<UStaticMeshComponent> CabinMesh = nullptr;
	UPROPERTY(VisibleAnywhere, Category = "Vehicle")
	TObjectPtr<UStaticMeshComponent> WindshieldMesh = nullptr;
	UPROPERTY(VisibleAnywhere, Category = "Vehicle")
	TObjectPtr<UStaticMeshComponent> SpoilerMesh = nullptr;
	UPROPERTY(VisibleAnywhere, Category = "Vehicle")
	TObjectPtr<UStaticMeshComponent> FrontLightMesh = nullptr;
	UPROPERTY(VisibleAnywhere, Category = "Vehicle")
	TObjectPtr<UStaticMeshComponent> RearLightMesh = nullptr;
	UPROPERTY(VisibleAnywhere, Category = "Vehicle")
	TObjectPtr<UStaticMeshComponent> TrunkDeckMesh = nullptr;
	UPROPERTY(VisibleAnywhere, Category = "Vehicle")
	TObjectPtr<UStaticMeshComponent> RearBumperMesh = nullptr;
	UPROPERTY(VisibleAnywhere, Category = "Vehicle")
	TObjectPtr<UStaticMeshComponent> HoodMesh = nullptr;
	UPROPERTY(VisibleAnywhere, Category = "Vehicle")
	TObjectPtr<UStaticMeshComponent> NoseMesh = nullptr;
	UPROPERTY(VisibleAnywhere, Category = "Vehicle")
	TObjectPtr<UStaticMeshComponent> IntakeLeftMesh = nullptr;
	UPROPERTY(VisibleAnywhere, Category = "Vehicle")
	TObjectPtr<UStaticMeshComponent> IntakeRightMesh = nullptr;
	UPROPERTY(VisibleAnywhere, Category = "Vehicle")
	TObjectPtr<UStaticMeshComponent> FrontSplitterMesh = nullptr;
	UPROPERTY(VisibleAnywhere, Category = "Vehicle")
	TObjectPtr<UStaticMeshComponent> SideSkirtMesh = nullptr;
	UPROPERTY(VisibleAnywhere, Category = "Vehicle")
	TObjectPtr<UStaticMeshComponent> RearDiffuserMesh = nullptr;
	UPROPERTY(VisibleAnywhere, Category = "Vehicle")
	TObjectPtr<UStaticMeshComponent> MirrorLeftMesh = nullptr;
	UPROPERTY(VisibleAnywhere, Category = "Vehicle")
	TObjectPtr<UStaticMeshComponent> MirrorRightMesh = nullptr;
	UPROPERTY(VisibleAnywhere, Category = "Vehicle")
	TObjectPtr<UStaticMeshComponent> WheelFL = nullptr;
	UPROPERTY(VisibleAnywhere, Category = "Vehicle")
	TObjectPtr<UStaticMeshComponent> WheelFR = nullptr;
	UPROPERTY(VisibleAnywhere, Category = "Vehicle")
	TObjectPtr<UStaticMeshComponent> WheelRL = nullptr;
	UPROPERTY(VisibleAnywhere, Category = "Vehicle")
	TObjectPtr<UStaticMeshComponent> WheelRR = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "Vehicle")
	TObjectPtr<UChaosWheeledVehicleMovementComponent> VehicleMovement = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TObjectPtr<USpringArmComponent> SpringArm = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TObjectPtr<UCameraComponent> FollowCamera = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Vehicle")
	FText DisplayName = FText::FromString(TEXT("Ciclón K-77"));

	/** Set true when the vehicle is being driven (skips idle physics sleep logic). */
	bool bBeingDriven = false;

	/** Pawn (the player character) that was driving; re-possessed on exit. */
	UPROPERTY()
	TObjectPtr<APawn> PreviousPawn = nullptr;

private:
	void BuildOriginalBodyVisual();
	void EnsureWheels();
	float LastImpactTime = -10.0f;
	static constexpr float ImpactCooldown = 0.6f;
};
