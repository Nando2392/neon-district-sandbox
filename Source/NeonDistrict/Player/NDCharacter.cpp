// Copyright Neon District Sandbox. Public benchmark repo — original content only.

#include "Player/NDCharacter.h"
#include "Core/NDPerfConstants.h"
#include "Core/NDGameInstance.h"
#include "Audio/NDAudioManager.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/InputComponent.h"

ANDCharacter::ANDCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(GetRootComponent());
	SpringArm->TargetArmLength = NDPerf::CameraSpringArmLength;
	SpringArm->bUsePawnControlRotation = true;
	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagSpeed = 8.0f;
	SpringArm->bDoCollisionTest = true;
	SpringArm->ProbeSize = NDPerf::CameraCollisionProbeSize;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = NDPerf::WalkSpeed;
		MoveComp->MaxWalkSpeedCrouched = NDPerf::WalkSpeed * 0.5f;
		MoveComp->JumpZVelocity = NDPerf::JumpVelocity;
		MoveComp->AirControl = 0.35f;
		MoveComp->bOrientRotationToMovement = true;
		MoveComp->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	}
}

void ANDCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	AccumulateFootstepDistance(DeltaSeconds);
}

void ANDCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	// Movement is driven entirely from the controller's Enhanced Input actions.
}

void ANDCharacter::SetSprinting(bool bNewSprinting)
{
	this->bSprinting = bNewSprinting;
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = bNewSprinting ? NDPerf::SprintSpeed : NDPerf::RunSpeed;
	}
}

void ANDCharacter::AccumulateFootstepDistance(float DeltaSeconds)
{
	const float Speed = GetVelocity().Size2D();
	if (Speed < 40.0f)
	{
		return; // idle/air: no steps
	}

	WalkedDistance += Speed * DeltaSeconds;
	const float Interval = bSprinting ? FootstepIntervalCm * 1.5f : FootstepIntervalCm;
	if (WalkedDistance >= Interval)
	{
		WalkedDistance = 0.0f;
		PlayFootstep();
	}
}

void ANDCharacter::PlayFootstep()
{
	if (UNDGameInstance* GI = GetGameInstance<UNDGameInstance>())
	{
		if (UNDAudioManager* Audio = GI->GetAudioManager())
		{
			Audio->PlayFootstep(this);
		}
	}
}
