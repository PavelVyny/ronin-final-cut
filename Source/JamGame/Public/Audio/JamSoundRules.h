#pragma once

#include "Core/JamTypes.h"
#include "CoreMinimal.h"

/** The sound effects of the fight, one per thing the player has to hear. */
enum class EJamCue : uint8
{
	KatanaSlash,
	ShurikenThrow,
	Explosion,
	Slam,
	Dash,
	Parry,
	Ultimate,
	BossHit,
	HeroHurt,
	StealCard,
	Steal,
	EverythingBack,
	FinalBlow,
	/** The hero's Ultimate is full (T-037): one more than the 13 names of Q8. */
	UltimateReady,
	/** The hero took the Publisher's cash heal (T-049): a cash register. */
	HealCash
};

/** How loud a cue plays and after how many seconds it fades out (0: plays whole). */
struct FJamMix
{
	float Volume = 1.f;
	float CutAfter = 0.f;
};

/**
 * Which sound a step of the fight makes. Presentation only: the simulation never asks. The sounds are
 * /Game/Audio/Sfx/<name>: stubs from Tools/Audio/make_stub_sfx.py until Kirill's files replace them under the same names.
 * Tests: Jam.Sound.*
 */
namespace JamSoundRules
{
inline constexpr int32 CueCount = 15;

/** The asset and file name of a cue, the way Kirill names his delivery: sfx_katana_slash_01 and so on. */
JAMGAME_API const TCHAR* NameOf(EJamCue Cue);

/** T-085: the crack's cue (the steal) plays short and quiet, and quieter still under a voice line. */
JAMGAME_API FJamMix MixOf(EJamCue Cue, bool bVoiceSpeaking);

/**
 * The cues of a frame's events, Fight being the state after them. Each cue sounds once per frame however many events
 * call it, in the order they first appear.
 */
JAMGAME_API TArray<EJamCue> CuesOf(const TArray<FJamEvent>& Events, const FJamFight& Fight);
} // namespace JamSoundRules
