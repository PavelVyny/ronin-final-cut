#include "Audio/JamMusic.h"

#include "Components/AudioComponent.h"
#include "Sim/JamLayerRules.h"
#include "Sound/SoundWave.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
// A cut takes a stem away over this time, the stun brings them back as fast: presentation only.
constexpr float FadeSeconds = 0.6f;
constexpr float MusicVolume = 0.8f;

const TCHAR* const StemPaths[] = {
	TEXT("/Game/Audio/Music/mus_main_1_drums.mus_main_1_drums"),
	TEXT("/Game/Audio/Music/mus_main_2_bass.mus_main_2_bass"),
	TEXT("/Game/Audio/Music/mus_main_3_pad.mus_main_3_pad"),
	TEXT("/Game/Audio/Music/mus_main_4_lead.mus_main_4_lead"),
};
const TCHAR* const MetronomePath = TEXT("/Game/Audio/Music/mus_poc_loop.mus_poc_loop");
const TCHAR* const AmbientPath = TEXT("/Game/Audio/Music/mus_ambient.mus_ambient");
} // namespace

AJamMusic::AJamMusic()
{
	PrimaryActorTick.bCanEverTick = true;
	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
	static_assert(UE_ARRAY_COUNT(StemPaths) == JamLayerRules::StemCount, "one sound per stem");
	for (int32 Index = 0; Index < JamLayerRules::StemCount; ++Index)
	{
		Tracks.Add(AddTrack(*FString::Printf(TEXT("Stem%d"), Index + 1), StemPaths[Index]));
	}
	Metronome = AddTrack(TEXT("Metronome"), MetronomePath);
	Ambient = AddTrack(TEXT("Ambient"), AmbientPath);
	Stems = JamLayerRules::StemCount;
}

UAudioComponent* AJamMusic::AddTrack(const TCHAR* Name, const TCHAR* Path)
{
	UAudioComponent* Track = CreateDefaultSubobject<UAudioComponent>(Name);
	Track->SetupAttachment(RootComponent);
	Track->bAutoActivate = false;
	Track->bIsUISound = true;
	ConstructorHelpers::FObjectFinder<USoundWave> Finder(Path);
	Track->SetSound(Finder.Object);
	return Track;
}

void AJamMusic::BeginPlay()
{
	Super::BeginPlay();
	Volumes.Init(0.f, Tracks.Num() + 1);
	TArray<UAudioComponent*> All(Tracks);
	All.Add(Metronome);
	All.Add(Ambient);
	for (int32 Index = 0; Index < All.Num(); ++Index)
	{
		if (USoundWave* Wave = Cast<USoundWave>(All[Index]->Sound))
		{
			// Imported as plain sounds: the loop and the silent play are what keeps the stems on one beat.
			Wave->bLooping = true;
			Wave->VirtualizationMode = EVirtualizationMode::PlayWhenSilent;
		}
		if (Index < Volumes.Num())
		{
			Volumes[Index] = Index < Stems || (Index == Tracks.Num() && Stems == 0) ? 1.f : 0.f;
		}
	}
	ApplyVolumes();
	for (UAudioComponent* Track : All)
	{
		Track->Play();
	}
}

void AJamMusic::ApplyVolumes()
{
	for (int32 Index = 0; Index < Volumes.Num(); ++Index)
	{
		UAudioComponent* Track = Index < Tracks.Num() ? Tracks[Index].Get() : Metronome.Get();
		Track->SetVolumeMultiplier(Volumes[Index] * Theme * MusicVolume * Gain);
	}
	Ambient->SetVolumeMultiplier((1.f - Theme) * MusicVolume * Gain);
}

void AJamMusic::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	const float Step = DeltaSeconds / FadeSeconds;
	for (int32 Index = 0; Index < Volumes.Num(); ++Index)
	{
		const bool bOn = Index < Tracks.Num() ? Index < Stems : Stems == 0;
		Volumes[Index] = FMath::Clamp(Volumes[Index] + (bOn ? Step : -Step), 0.f, 1.f);
	}
	ApplyVolumes();
}

void AJamMusic::SetTheme(float Level)
{
	Theme = FMath::Clamp(Level, 0.f, 1.f);
}

void AJamMusic::SetGain(float InGain)
{
	Gain = InGain;
	ApplyVolumes();
}
