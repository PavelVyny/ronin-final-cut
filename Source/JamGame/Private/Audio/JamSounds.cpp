#include "Audio/JamSounds.h"

#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Sound/SoundWave.h"
#include "UObject/ConstructorHelpers.h"

AJamSounds::AJamSounds()
{
	PrimaryActorTick.bCanEverTick = false;
	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
	for (int32 Index = 0; Index < JamSoundRules::CueCount; ++Index)
	{
		const TCHAR* Name = JamSoundRules::NameOf(static_cast<EJamCue>(Index));
		ConstructorHelpers::FObjectFinder<USoundWave> Finder(*FString::Printf(TEXT("/Game/Audio/Sfx/%s.%s"), Name, Name));
		Sounds.Add(Finder.Object);
	}
	// In EJamComicSound's order after None.
	for (const TCHAR* Name : { TEXT("sfx_comic_typing_01"), TEXT("sfx_comic_ring_01"), TEXT("sfx_comic_shatter_01") })
	{
		ConstructorHelpers::FObjectFinder<USoundWave> Finder(*FString::Printf(TEXT("/Game/Audio/Sfx/%s.%s"), Name, Name));
		ComicSounds.Add(Finder.Object);
	}
}

void AJamSounds::PlayComic(int32 Index, float Volume)
{
	if (ComicPlaying)
	{
		ComicPlaying->Stop();
		ComicPlaying = nullptr;
	}
	if (ComicSounds.IsValidIndex(Index) && ComicSounds[Index])
	{
		ComicPlaying = UGameplayStatics::SpawnSound2D(this, ComicSounds[Index], Volume);
	}
}

void AJamSounds::Play(const TArray<FJamEvent>& Events, const FJamFight& Fight, bool bVoiceSpeaking)
{
	for (const EJamCue Cue : JamSoundRules::CuesOf(Events, Fight))
	{
		USoundWave* Sound = Sounds[static_cast<int32>(Cue)];
		if (!Sound)
		{
			continue;
		}
		const FJamMix Mix = JamSoundRules::MixOf(Cue, bVoiceSpeaking);
		UAudioComponent* Playing = UGameplayStatics::SpawnSound2D(this, Sound, Mix.Volume);
		if (Playing && Mix.CutAfter > 0.f)
		{
			CutShort(*Playing, Mix.CutAfter);
		}
	}
}

void AJamSounds::CutShort(UAudioComponent& Playing, float After)
{
	// A quick fade, not a click.
	TWeakObjectPtr<UAudioComponent> Weak(&Playing);
	FTimerHandle Cut;
	GetWorldTimerManager().SetTimer(
		Cut,
		[Weak]()
		{
			if (Weak.IsValid())
			{
				Weak->FadeOut(0.1f, 0.f);
			}
		},
		After, false);
}
