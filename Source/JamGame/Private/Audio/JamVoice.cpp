#include "Audio/JamVoice.h"

#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundWave.h"
#include "UObject/ConstructorHelpers.h"

AJamVoice::AJamVoice()
{
	PrimaryActorTick.bCanEverTick = true;
	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
	for (int32 Index = 0; Index < JamVoiceRules::LineCount; ++Index)
	{
		const TCHAR* Name = JamVoiceRules::NameOf(static_cast<EJamLine>(Index));
		ConstructorHelpers::FObjectFinder<USoundWave> Finder(*FString::Printf(TEXT("/Game/Audio/Voice/%s.%s"), Name, Name));
		Sounds.Add(Finder.Object);
	}
	Stream.Initialize(FPlatformTime::Cycles());
}

void AJamVoice::Say(EJamLine NewLine, bool bNow)
{
	if (bNow || JamVoiceRules::CutsIn(NewLine))
	{
		Hush();
	}
	CutBanterFor(NewLine);
	Queue.Add(NewLine);
	if (!bSpeaking)
	{
		StartNext(GetWorld()->GetRealTimeSeconds());
	}
}

void AJamVoice::Hush()
{
	Queue.Reset();
	Held.Reset();
	bSpeaking = false;
	if (Playing)
	{
		Playing->Stop();
		Playing = nullptr;
	}
}

void AJamVoice::SetGain(float InGain)
{
	Gain = InGain;
	if (Playing)
	{
		Playing->SetVolumeMultiplier(JamVoiceRules::Loudness(Gain));
	}
}

void AJamVoice::CutBanterFor(EJamLine Incoming)
{
	Queue.RemoveAll([Incoming](EJamLine Queued) { return JamVoiceRules::CutsOff(Queued, Incoming); });
	if (bSpeaking && JamVoiceRules::CutsOff(Line, Incoming))
	{
		bSpeaking = false;
		if (Playing)
		{
			Playing->Stop();
			Playing = nullptr;
		}
	}
}

bool AJamVoice::Current(EJamLine& OutLine) const
{
	OutLine = Line;
	return bSpeaking;
}

void AJamVoice::StartNext(float Now)
{
	bSpeaking = false;
	while (!Queue.IsEmpty() && !bSpeaking)
	{
		Line = Queue[0];
		Queue.RemoveAt(0);
		USoundWave* Sound = Sounds[static_cast<int32>(Line)];
		if (!Sound)
		{
			continue;
		}
		Playing = UGameplayStatics::SpawnSound2D(this, Sound, JamVoiceRules::Loudness(Gain));
		LineEnd = Now + Sound->GetDuration();
		bSpeaking = true;
	}
	if (!bSpeaking)
	{
		Playing = nullptr;
		SilentSince = Now;
	}
}

void AJamVoice::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	const float Now = GetWorld()->GetRealTimeSeconds();
	if (bSpeaking && Now >= LineEnd)
	{
		StartNext(Now);
	}
}

void AJamVoice::Hear(const TArray<FJamEvent>& Events, const FJamFight& Fight, bool bCracking)
{
	for (const EJamLine NewLine : JamVoiceRules::LinesOf(Events, Fight))
	{
		Say(NewLine);
	}
	const float Now = GetWorld()->GetRealTimeSeconds();
	Held.Append(JamVoiceRules::CommentsOf(Events));
	const int32 Queued = Queue.Num();
	JamVoiceRules::Release(Held, Queue, bCracking);
	if (Queue.Num() > Queued)
	{
		// A comment let out of the hold is an event line too: it stops the banter.
		const EJamLine Released = Queue[Queued];
		CutBanterFor(Released);
	}
	if (!Queue.IsEmpty() && !bSpeaking)
	{
		StartNext(Now);
	}
	if (SilentSince < 0.f || bSpeaking)
	{
		SilentSince = bSpeaking ? -1.f : Now;
		return;
	}
	if (JamVoiceRules::MayFill(Fight, bSpeaking, !Held.IsEmpty(), Now - SilentSince))
	{
		for (const EJamLine Filler : JamVoiceRules::NextFiller(Stream, FillerBag, LastFiller))
		{
			Say(Filler);
		}
	}
}
