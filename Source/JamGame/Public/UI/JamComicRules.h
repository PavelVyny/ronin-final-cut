#pragma once

#include "Audio/JamVoiceRules.h"
#include "CoreMinimal.h"

/** The sound under a comic panel (the session-5s playtest). */
enum class EJamComicSound : uint8
{
	None,
	/** The dev at his mechanical keyboard. */
	Typing,
	/** The Publisher's call coming in. */
	Ring,
	/** His gauntlet smashing through the monitor. */
	Shatter
};

/** One panel of the opening comic: its picture's shape, the camera's move over it and the lines said over it. */
struct FJamComicPanel
{
	/** The least the panel stays up, and how long the camera's move takes. */
	float Seconds = 3.f;
	/** The picture's width over its height. */
	float Aspect = 1.5f;
	/**
	 * Where the camera looks on the picture (0 to 1 both ways, Y down) and how close (1: the largest part of the picture
	 * the screen's shape fits), as the panel starts and when the move ends.
	 */
	FVector2D From = FVector2D(0.5, 0.5);
	float FromZoom = 1.f;
	FVector2D To = FVector2D(0.5, 0.5);
	float ToZoom = 1.f;
	/** The voice lines said over it, in order. */
	TArray<EJamLine> Lines;
	/** The sound that starts with it. */
	EJamComicSound Sound = EJamComicSound::None;
};

/**
 * The opening comic after NEW GAME (T-093, Art/Comic/COMIC_SCRIPT.md): five panels, each sliding in from the right,
 * the camera panning and pushing in over it, the voiced lines in balloons. A panel waits for its lines; a key skips the
 * comic after a breath, so the menu's Enter does not.
 * Tests: Jam.Comic.*
 */
namespace JamComicRules
{
inline constexpr int32 PanelCount = 5;
inline constexpr float SlideSeconds = 0.3f;
inline constexpr float SkipAfter = 0.3f;
/** T-094: the comic's own shape; a wider screen gets black bars at the sides. */
inline constexpr float FrameAspect = 16.f / 9.f;
/** The call's ring (sfx_comic_ring_01) lasts this long; the Publisher speaks after it. */
inline constexpr float RingSeconds = 2.5f;
/** The first panel fades up from black over this long. */
inline constexpr float FadeSeconds = 0.8f;

JAMGAME_API const FJamComicPanel& PanelOf(int32 Index);

/**
 * T-100: how loud a panel's sound plays (1: as recorded). The glass of the gauntlet's panel is quiet, under the
 * Publisher's line that starts with it, but clearly heard.
 */
JAMGAME_API float SoundVolume(EJamComicSound Sound);

/** The panel has had its time and nobody is talking any more. */
JAMGAME_API bool IsPanelDone(int32 Index, float PanelAge, bool bSpeaking);

/** Where the comic shows on a screen of Screen pixels: centred, at most FrameAspect wide, the full height. */
JAMGAME_API FBox2D Frame(const FVector2D& Screen);

/** The part of the panel's picture in a frame of this shape, PanelAge into it: UVs, eased from From to To. */
JAMGAME_API FBox2D Crop(const FJamComicPanel& Panel, float PanelAge, float ScreenAspect);

/** How far a panel has slid in from the right: 0 as it starts, 1 in place. */
JAMGAME_API float SlideIn(float PanelAge);

/** Whether panel Index slides in over the last one; the first has none to slide over and fades up instead. */
JAMGAME_API bool SlidesIn(int32 Index);

/** How far panel Index is up from black, PanelAge into it: 0 to 1, 1 for a panel that slides in. */
JAMGAME_API float FadeIn(int32 Index, float PanelAge);

/** A key may skip the comic this long after it started. */
JAMGAME_API bool MaySkip(float Age);

/** The words of a balloon in rows of at most MaxChars, a longer word on a row of its own. */
JAMGAME_API TArray<FString> Wrap(const FString& Text, int32 MaxChars);

/** The centre of a line's balloon in the frame, as fractions of it, over the empty dark part of its panel. */
JAMGAME_API FVector2D BalloonAt(EJamLine Line);
} // namespace JamComicRules
