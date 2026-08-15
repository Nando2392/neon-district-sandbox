// Copyright Neon District Sandbox. Public benchmark repo — original content only.

#include "AI/NDNPCAIController.h"
#include "Core/NDPerfConstants.h"
#include "Systems/NDWantedSystem.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NavigationSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

ANDNPCAIController::ANDNPCAIController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ANDNPCAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	if (ACharacter* NPC = Cast<ACharacter>(InPawn))
	{
		if (UCharacterMovementComponent* MoveComp = NPC->GetCharacterMovement())
		{
			MoveComp->MaxWalkSpeed = bPolice ? NDPerf::PoliceChaseSpeed * 0.6f : 170.0f;
		}
	}
}

void ANDNPCAIController::SetPatrolPoints(const TArray<FVector>& Points, bool bInPolice)
{
	PatrolPoints = Points;
	bPolice = bInPolice;
	Behavior = bPolice ? ENPNPCBehavior::ReturnToPatrol : ENPNPCBehavior::PatrolCivilian;
	NextPatrolIndex = 0;

	if (ACharacter* NPC = Cast<ACharacter>(GetPawn()))
	{
		if (UCharacterMovementComponent* MoveComp = NPC->GetCharacterMovement())
		{
			MoveComp->MaxWalkSpeed = bPolice ? NDPerf::PoliceChaseSpeed * 0.6f : 170.0f;
		}
	}
}

void ANDNPCAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	switch (Behavior)
	{
	case ENPNPCBehavior::PatrolCivilian:
		UpdatePatrol(DeltaSeconds);
		break;
	case ENPNPCBehavior::FleeCivilian:
		UpdateFlee(DeltaSeconds);
		break;
	case ENPNPCBehavior::ChasePlayer:
	case ENPNPCBehavior::SearchForPlayer:
	case ENPNPCBehavior::ReturnToPatrol:
		UpdatePolice(DeltaSeconds);
		break;
	}
}

bool ANDNPCAIController::CanSeePlayer(ACharacter* Player, FVector& OutViewPoint) const
{
	if (!Player)
	{
		return false;
	}

	OutViewPoint = Player->GetActorLocation() + FVector(0.0f, 0.0f, 120.0f);
	const FVector EyeLocation = GetPawn()->GetActorLocation() + FVector(0.0f, 0.0f, 150.0f);

	FCollisionQueryParams Params(SCENE_QUERY_STAT(NDPoliceVision), true);
	Params.AddIgnoredActor(GetPawn());

	FHitResult Hit;
	if (GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, OutViewPoint, ECC_Visibility, Params))
	{
		return Hit.GetActor() == Player;
	}
	return true;
}

void ANDNPCAIController::UpdatePatrol(float DeltaSeconds)
{
	if (PatrolWaitTimer > 0.0f)
	{
		PatrolWaitTimer -= DeltaSeconds;
		StopMovement();
		return;
	}

	if (PatrolPoints.Num() == 0)
	{
		// Stand-in: wander around home.
		FNavLocation NavLocation;
		UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
		if (NavSys && NavSys->GetRandomReachablePointInRadius(GetPawn()->GetActorLocation(), 600.0f, NavLocation))
		{
			PatrolPoints.Add(NavLocation);
			NextPatrolIndex = 0;
		}
		else
		{
			return;
		}
	}

	const FVector Target = PatrolPoints[NextPatrolIndex % PatrolPoints.Num()];
	const float DistSq = FVector::DistSquared(Target, GetPawn()->GetActorLocation());
	if (DistSq < 40.0f * 40.0f)
	{
		++NextPatrolIndex;
		PatrolWaitTimer = PatrolWaitSeconds;
		return;
	}

	MoveToLocation(Target, 30.0f);

	// Civilians flee briefly when heat is live and the player is very close.
	if (UNDWantedSystem* Wanted = GetGameInstance()->GetSubsystem<UNDWantedSystem>())
	{
		if (Wanted->GetWantedLevel() >= 1)
		{
			if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
			{
				if (ACharacter* Player = Cast<ACharacter>(PC->GetPawn()))
				{
					const float Dist = FVector::Dist(Player->GetActorLocation(), GetPawn()->GetActorLocation());
					if (Dist < 500.0f)
					{
						FVector Away = GetPawn()->GetActorLocation() - Player->GetActorLocation();
						Away.Z = 0.0f;
						Away.Normalize();
						const FVector FleePoint = GetPawn()->GetActorLocation() + Away * 800.0f;
						PatrolPoints.Insert(FleePoint, NextPatrolIndex + 1);
						Behavior = ENPNPCBehavior::FleeCivilian;
						FleeTimer = FleeSeconds;
						if (UCharacterMovementComponent* MoveComp = Cast<ACharacter>(GetPawn())->GetCharacterMovement())
						{
							MoveComp->MaxWalkSpeed = 420.0f;
						}
					}
				}
			}
		}
	}
}

void ANDNPCAIController::UpdateFlee(float DeltaSeconds)
{
	FleeTimer -= DeltaSeconds;
	if (FleeTimer <= 0.0f)
	{
		Behavior = ENPNPCBehavior::PatrolCivilian;
		if (UCharacterMovementComponent* MoveComp = Cast<ACharacter>(GetPawn())->GetCharacterMovement())
		{
			MoveComp->MaxWalkSpeed = 170.0f;
		}
		return;
	}
	// Keep moving toward the flee point until the timer expires.
	if (PatrolPoints.Num() > 0)
	{
		MoveToLocation(PatrolPoints[NextPatrolIndex % PatrolPoints.Num()], 30.0f);
	}
}

void ANDNPCAIController::UpdatePolice(float DeltaSeconds)
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	ACharacter* Player = PC ? Cast<ACharacter>(PC->GetPawn()) : nullptr;
	UNDWantedSystem* Wanted = GetGameInstance()->GetSubsystem<UNDWantedSystem>();

	FVector PlayerEye;
	const bool bCanSee = CanSeePlayer(Player, PlayerEye);
	const float Dist = Player ? FVector::Dist(Player->GetActorLocation(), GetPawn()->GetActorLocation()) : 0.0f;

	if (Behavior == ENPNPCBehavior::ChasePlayer)
	{
		if (!Player || Dist > NDPerf::PoliceLoseRadius)
		{
			// Lost entirely (too far): drop pursuit, decay heat.
			Behavior = ENPNPCBehavior::ReturnToPatrol;
			if (Wanted)
			{
				Wanted->ReportEvasion();
			}
			return;
		}

		if (bCanSee)
		{
			LoseSightTimer = 0.0f;
			if (Wanted)
			{
				Wanted->ReportDetection(0.6f); // sustained sighting builds heat
			}
		}
		else
		{
			LoseSightTimer += DeltaSeconds;
			if (Wanted)
			{
				Wanted->ReportEvasion();
			}
			if (LoseSightTimer >= NDPerf::PoliceLoseDelay)
			{
				Behavior = ENPNPCBehavior::ReturnToPatrol;
				return;
			}
			// Last known position search.
			Behavior = ENPNPCBehavior::SearchForPlayer;
			MoveToLocation(PlayerEye, 60.0f);
			return;
		}

		// Chase with escalating speed by wanted level.
		const float Speed = 480.0f + (Wanted ? Wanted->GetWantedLevel() * 90.0f : 0.0f);
		if (UCharacterMovementComponent* MoveComp = Cast<ACharacter>(GetPawn())->GetCharacterMovement())
		{
			MoveComp->MaxWalkSpeed = Speed;
		}
		MoveToActor(Player, 120.0f);
		return;
	}

	if (Behavior == ENPNPCBehavior::SearchForPlayer)
	{
		if (bCanSee && Player)
		{
			Behavior = ENPNPCBehavior::ChasePlayer;
			if (Wanted)
			{
				Wanted->ReportDetection(1.0f);
			}
			return;
		}
		LoseSightTimer += DeltaSeconds;
		if (LoseSightTimer >= NDPerf::PoliceLoseDelay)
		{
			Behavior = ENPNPCBehavior::ReturnToPatrol;
		}
		return;
	}

	// ReturnToPatrol: patrol normally, but engage when the player is detected.
	if (bCanSee && Player && Dist <= NDPerf::PoliceDetectionRadius && Wanted)
	{
		Behavior = ENPNPCBehavior::ChasePlayer;
		Wanted->ReportDetection(1.0f);
		return;
	}

	UpdatePatrol(DeltaSeconds);
}
