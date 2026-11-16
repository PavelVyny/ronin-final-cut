#pragma once

#include "Audio/JamSoundRules.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "JamSounds.generated.h"

class USoundWave;

/**
 * The sound effects: plays the cues JamSoundRules picks from a frame's events. Every sound is heard at the same
 * loudness wherever it happens: the camera is above the hero and the arena is small.
 *
 * The sounds are /Game/Audio/Sfx/<cue name>: stubs from Tools/Audio/make_stub_sfx.py until Kirill's files replace
 * them under the same names.
 */
UCLASS()
class JAMGAME_API AJamSounds : public AActor
{
	GENERATED_BODY()

public:
	AJamSounds();

	/** Plays the cues of a frame's events, Fight being the state after them, ducked under a voice line (T-085). */
	void Play(const TArray<FJamEvent>& Events, const FJamFight& Fight, bool bVoiceSpeaking);

	/**
	 * Starts the opening comic's sound Index (0 the typing, 1 the ring: EJamComicSound after None) and stops the one
	 * before; INDEX_NONE only stops it.
	 */
	void PlayComic(int32 Index, float Volume = 1.f);

private:
	/** Fades Playing out After seconds in (T-085). */
	void CutShort(class UAudioComponent& Playing, float After);

public:
private:
	/** One sound per cue, in the order of EJamCue; empty where an asset is missing. */
	UPROPERTY()
	TArray<TObjectPtr<USoundWave>> Sounds;

	/** The opening comic's sounds, and the one playing. */
	UPROPERTY()
	TArray<TObjectPtr<USoundWave>> ComicSounds;
	UPROPERTY()
	TObjectPtr<class UAudioComponent> ComicPlaying;
};
