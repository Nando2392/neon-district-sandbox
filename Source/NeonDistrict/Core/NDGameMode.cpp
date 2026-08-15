// Copyright Neon District Sandbox. Public benchmark repo — original content only.

#include "Core/NDGameMode.h"
#include "Player/NDCharacter.h"
#include "Player/NDPlayerController.h"
#include "Systems/NDWorldBuilder.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

ANDGameMode::ANDGameMode()
{
	DefaultPawnClass = ANDCharacter::StaticClass();
	PlayerControllerClass = ANDPlayerController::StaticClass();
}

void ANDGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Menu level has no player pawn (the menu widget covers the viewport).
	if (GetWorld() && GetWorld()->GetName() == TEXT("ND_MainMenu"))
	{
		DefaultPawnClass = nullptr;
	}
}

void ANDGameMode::RestartPlayer(AController* NewPlayer)
{
	Super::RestartPlayer(NewPlayer);

	// The district's default spawn (0,0,0) sits inside the central block of
	// procedural buildings. Reposition the freshly-spawned pawn onto a real
	// avenue so the player starts on open asphalt and can actually move.
	if (!NewPlayer || !GetWorld())
	{
		return;
	}
	APawn* Pawn = NewPlayer->GetPawn();
	if (!Pawn)
	{
		return;
	}
	AActor* Builder = UGameplayStatics::GetActorOfClass(GetWorld(), ANDWorldBuilder::StaticClass());
	ANDWorldBuilder* WorldBuilder = Cast<ANDWorldBuilder>(Builder);
	if (!WorldBuilder)
	{
		return;
	}
	const FVector StreetPoint = WorldBuilder->GetRandomStreetPoint();
	const FVector TraceStart = StreetPoint + FVector(0.0f, 0.0f, 800.0f);
	const FVector TraceEnd = StreetPoint - FVector(0.0f, 0.0f, 400.0f);

	FHitResult Hit;
	FCollisionQueryParams QueryParams(FName(TEXT("ND_SpawnPlacement")), /*bTraceComplex=*/false);
	QueryParams.AddIgnoredActor(Pawn);
	if (GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams) && Hit.bBlockingHit)
	{
		const UCapsuleComponent* Capsule = Pawn->FindComponentByClass<UCapsuleComponent>();
		const float CapsuleHalf = Capsule ? Capsule->GetScaledCapsuleHalfHeight() : 90.0f;
		const FVector SpawnLoc(Hit.ImpactPoint.X, Hit.ImpactPoint.Y, Hit.ImpactPoint.Z + CapsuleHalf + 5.0f);
		Pawn->SetActorLocation(SpawnLoc, /*bSweep=*/false, nullptr, ETeleportType::TeleportPhysics);
		if (UCharacterMovementComponent* MoveComp = Pawn->FindComponentByClass<UCharacterMovementComponent>())
		{
			MoveComp->SetMovementMode(MOVE_Walking);
		}
		UE_LOG(LogTemp, Log, TEXT("NeonDistrict: player spawned on avenue at (%.0f, %.0f, %.0f)"),
			SpawnLoc.X, SpawnLoc.Y, SpawnLoc.Z);
	}
}
