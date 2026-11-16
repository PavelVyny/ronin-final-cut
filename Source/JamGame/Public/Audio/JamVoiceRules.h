#pragma once

#include "Core/JamTypes.h"
#include "CoreMinimal.h"

/**
 * The voice-over lines (Audio/Eleven/Voice/VO_LINES.md), in the order of their files: the intro, the exchange of each
 * cut, the stun, the final blow, the Publisher's barks, his comments on the moves he steals, the opening comic's call.
 */
enum class EJamLine : uint8
{
	IntroBoss,
	IntroHero,
	Cut1Boss,
	Cut1Hero,
	Cut2Boss,
	Cut2Hero,
	Cut3Boss,
	Cut3Hero,
	Cut4Boss,
	Cut4Hero,
	StunBoss,
	FinalHero,
	FinalBoss,
	Bark1,
	Bark2,
	Bark3,
	Bark4,
	StealShuriken,
	StealParry,
	StealDashBoss,
	StealDashHero,
	StealUltimate,
	ComicBoss1,
	ComicHero1,
	ComicBoss2,
	ComicHero2,
	Banter1Boss,
	Banter1Hero,
	Banter2Boss,
	Banter2Hero,
	Banter3Boss,
	Banter3Hero,
	Banter4Boss,
	Banter4Hero,
	Banter5Boss,
	Banter5Hero,
	Banter6Boss,
	Banter6Hero,
	Banter7Boss,
	Banter7Hero,
	Banter8Boss,
	Banter8Hero
};

/**
 * Who says what when. Presentation only: the simulation never asks. The sounds are /Game/Audio/Voice/<name>, made with
 * ElevenLabs by Tools/Audio/eleven_audio.py speak.
 * Tests: Jam.Voice.*
 */
namespace JamVoiceRules
{
inline constexpr int32 LineCount = 42;
inline constexpr int32 BarkCount = 4;
/** Pairs of banter: the Publisher's line and Ronin's answer. */
inline constexpr int32 BanterCount = 8;
/** What fills a quiet fight: a pair of banter or one of the old barks. */
inline constexpr int32 FillerCount = BanterCount + BarkCount;
/** A quiet fight is filled once no line has played for longer than this. */
inline constexpr float SilenceSeconds = 5.f;
/** The Publisher barks now and then in the fight: this long after the last line ends. */
inline constexpr float BarkMinSeconds = 15.f;
inline constexpr float BarkMaxSeconds = 25.f;
/** T-094: the lines were too quiet under the music: this much louder than recorded at full voice volume. */
inline constexpr float Boost = 1.3f;

/** The asset and file name of a line: vo_intro_pub and so on, _pub for the Publisher, _ron for the hero. */
JAMGAME_API const TCHAR* NameOf(EJamLine Line);

/** Who says it: the Boss side is the Publisher. */
JAMGAME_API EJamSide SpeakerOf(EJamLine Line);

JAMGAME_API bool IsBark(EJamLine Line);

/** A line of a pair of banter (the session-5s playtest). */
JAMGAME_API bool IsBanter(EJamLine Line);

/** The fight has gone quiet for more than SilenceSeconds: in the plain fight, nobody talking, no comment waiting. */
JAMGAME_API bool MayFill(const FJamFight& Fight, bool bSpeaking, bool bHolding, float Silence);

/**
 * What fills the quiet next: a pair of banter (the Publisher, then Ronin) or one bark, drawn from Bag without a repeat
 * until every one has played; the bag refills without Last, the one just played, so nothing comes twice running.
 */
JAMGAME_API TArray<EJamLine> NextFiller(FRandomStream& Stream, TArray<int32>& Bag, int32& Last);

/** Whether Incoming stops Saying at once: an event line cuts the banter off. */
JAMGAME_API bool CutsOff(EJamLine Saying, EJamLine Incoming);

/**
 * The lines a frame's events start, in the order they are said, Fight being the state after them: a steal card opens
 * the exchange of its cut, the stun his bargain, the final blow the hero's last word and the Publisher's gasp.
 */
JAMGAME_API TArray<EJamLine> LinesOf(const TArray<FJamEvent>& Events, const FJamFight& Fight);

/** A line that stops whatever is being said and drops the queue: the final blow does not wait. */
JAMGAME_API bool CutsIn(EJamLine Line);

/** Seconds from the end of the last line to the next bark. */
JAMGAME_API float NextBarkDelay(FRandomStream& Stream);

/** A bark other than the last one. */
JAMGAME_API EJamLine PickBark(FRandomStream& Stream, EJamLine Last);

/**
 * The Publisher's comment on the moves a frame's events hand him (StealResolved), with the hero's answer to the Dash.
 * The Katana has none, so the last cut, which takes it with the one move left, gets the move's comment.
 */
JAMGAME_API TArray<EJamLine> CommentsOf(const TArray<FJamEvent>& Events);

/** Held comments join the queue, after the card's lines still being said, once the crack between two looks is over. */
JAMGAME_API void Release(TArray<EJamLine>& Held, TArray<EJamLine>& Queue, bool bCracking);

/** Barks only in the plain fight, never over another line, nor while a comment waits for the crack (bHolding). */
JAMGAME_API bool MayBark(const FJamFight& Fight, bool bSpeaking, bool bHolding = false);

/** How loud a line plays at the menu's voice volume MenuGain (0 to 1). */
JAMGAME_API float Loudness(float MenuGain);
} // namespace JamVoiceRules
