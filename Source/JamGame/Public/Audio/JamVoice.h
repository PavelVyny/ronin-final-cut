#pragma once

#include "Audio/JamVoiceRules.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "JamVoice.generated.h"

class UAudioComponent;
class USoundWave;

/**
 * The voice-over: says the lines JamVoiceRules picks one after another, never two at once, and fills a fight gone quiet
 * with banter or a bark (JamVoiceRules::MayFill, NextFiller); an event line cuts the banter off (CutsOff). Runs on real time, so a slowed world does not stretch a line. A line outlives the steal
 * card it started on. His comment on a stolen move waits for the crack between two looks and the card's lines.
 *
 * The sounds are /Game/Audio/Voice/<line name>; a missing one is skipped silently with its subtitle.
 */
UCLASS()
class JAMGAME_API AJamVoice : public AActor
{
	GENERATED_BODY()

public:
	AJamVoice();

	virtual void Tick(float DeltaSeconds) override;

	/** Queues Line, or says it at once over everything when it cuts in (JamVoiceRules::CutsIn) or bNow. */
	void Say(EJamLine Line, bool bNow = false);

	/**
	 * The lines of a frame's events, and the barks of the fight; Fight is the state after the events, bCracking whether the
	 * crack between two looks is still open.
	 */
	void Hear(const TArray<FJamEvent>& Events, const FJamFight& Fight, bool bCracking);

	/** Stops the line being said and drops the queue. */
	void Hush();

	/** The line being said now, false when nobody talks. */
	bool Current(EJamLine& OutLine) const;

	/** Whether a line is playing now: the sound effects duck under it. */
	bool IsSpeaking() const { return bSpeaking; }

	/** The player's voice volume from the menu (T-092), 0 to 1. */
	void SetGain(float InGain);

private:
	void StartNext(float Now);

	/** Stops the banter being said, and drops what is left of it, when Incoming cuts it off. */
	void CutBanterFor(EJamLine Incoming);

	UPROPERTY()
	TArray<TObjectPtr<USoundWave>> Sounds;

	UPROPERTY()
	TObjectPtr<UAudioComponent> Playing;

	TArray<EJamLine> Queue;
	/** Comments on a stolen move, waiting for the crack to close. */
	TArray<EJamLine> Held;
	bool bSpeaking = false;
	EJamLine Line = EJamLine::IntroBoss;
	/** Real time the line being said ends, and the last one ended (negative: not yet set). */
	float LineEnd = 0.f;
	float SilentSince = -1.f;
	/** The banter and barks still to play this round, and the one played last. */
	TArray<int32> FillerBag;
	int32 LastFiller = INDEX_NONE;
	FRandomStream Stream;
	float Gain = 1.f;
};
