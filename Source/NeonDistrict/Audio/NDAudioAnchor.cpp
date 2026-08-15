// Copyright Neon District Sandbox. Public benchmark repo — original content only.

#include "Audio/NDAudioAnchor.h"
#include "Audio/NDAudioManager.h"
#include "Audio/NDSynthAudioComponent.h"
#include "Core/NDGameInstance.h"
#include "Kismet/GameplayStatics.h"

ANDAudioAnchor::ANDAudioAnchor()
{
	PrimaryActorTick.bCanEverTick = false;

	Synth = CreateDefaultSubobject<UNDSynthAudioComponent>(TEXT("ProceduralSynth"));
	Synth->SetupAttachment(RootComponent);
	Synth->SetAutoActivate(true);
}

void ANDAudioAnchor::BeginPlay()
{
	Super::BeginPlay();

	if (UNDGameInstance* GI = GetGameInstance<UNDGameInstance>())
	{
		if (UNDAudioManager* AM = GI->GetAudioManager())
		{
			AM->RegisterSynth(Synth);
		}
	}
}
