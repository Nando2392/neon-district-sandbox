// Copyright Neon District Sandbox. Public benchmark repo — original content only.

#include "Core/NDGameMode.h"
#include "Player/NDCharacter.h"
#include "Player/NDPlayerController.h"

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
