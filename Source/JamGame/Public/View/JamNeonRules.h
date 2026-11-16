#pragma once

#include "CoreMinimal.h"
#include "Sim/JamLayerRules.h"

#include "JamNeonRules.generated.h"

/**
 * Numbers of the neon look of the stand-ins. The values live in the [JamNeon] section of Config/DefaultGame.ini, one
 * line per field. Glow is the multiplier of the neon material: above 1 it blooms.
 */
USTRUCT()
struct JAMGAME_API FJamNeonSettings
{
	GENERATED_BODY()

	/** The tempo of the music the floor lines pulse to. */
	UPROPERTY()
	float Bpm = 0.f;

	/** The glow of the floor lines between beats, the extra glow on a beat, and how fast the extra dies (seconds). */
	UPROPERTY()
	float LineGlow = 0.f;

	UPROPERTY()
	float BeatPulse = 0.f;

	UPROPERTY()
	float BeatDecay = 0.f;

	/** The steady glow of the posts and of the ring at the edge of the arena. */
	UPROPERTY()
	float PostGlow = 0.f;

	UPROPERTY()
	float EdgeGlow = 0.f;

	/** The glow of the effects: blades, flashes, warnings. */
	UPROPERTY()
	float EffectGlow = 0.f;

	/** Bloom of the camera with the full art, with the shape ronin's plain effects (T-077), and once the Publisher took
	 * the effects too. */
	UPROPERTY()
	float Bloom = 0.f;

	UPROPERTY()
	float BloomFlat = 0.f;

	UPROPERTY()
	float BloomCut = 0.f;

	/**
	 * T-094: the film look of the full art, so the generated textures' noise reads as atmosphere: grain, colour fringes
	 * at the edges (from FringeStart, 0 the centre to 1 the corners), rain haze as height fog from FogStart cm off the
	 * camera, and the far distance softened (FarBlurKm: kilometres to half the blur, FarBlurRadius: pixels at 1080p).
	 */
	UPROPERTY()
	float FilmGrain = 0.f;

	UPROPERTY()
	float Fringe = 0.f;

	UPROPERTY()
	float FringeStart = 0.f;

	UPROPERTY()
	float FogDensity = 0.f;

	UPROPERTY()
	float FogFalloff = 0.f;

	UPROPERTY()
	float FogStart = 0.f;

	UPROPERTY()
	float FogOpacity = 0.f;

	UPROPERTY()
	float FarBlurKm = 0.f;

	UPROPERTY()
	float FarBlurRadius = 0.f;
};

/** How much of the film look a tier gets (JamNeonRules::FilmLookFor); all zero means none. */
struct FJamFilmLook
{
	float Grain = 0.f;
	float Fringe = 0.f;
	float FringeStart = 0.f;
	float FogDensity = 0.f;
	float FogFalloff = 0.f;
	float FogStart = 0.f;
	float FogOpacity = 0.f;
	float FarBlurKm = 0.f;
	float FarBlurRadius = 0.f;
};

/**
 * The light and the post process of a tier (T-077): the full art gets Lumen, neon lights round the arena, bloom, a
 * vignette and a grade; the shape ronin a flat light with a hint of bloom; greybox the bare engine look.
 */
enum class EJamLight : uint8
{
	Neon,
	Flat,
	Bare
};

/**
 * The neon look: how bright the floor lines are at a moment of the music, and how much the camera blooms with the
 * layers the Publisher has left. Presentation only.
 * Tests: Jam.Neon.*
 */
namespace JamNeonRules
{
/** Section with the neon numbers (FJamNeonSettings). */
inline const TCHAR* const Section = TEXT("JamNeon");

/** The glow of the floor lines MusicTime seconds after the music started: a kick on every beat, then a fall. */
JAMGAME_API float LineGlow(float MusicTime, const FJamNeonSettings& Settings);

/**
 * The bloom of the camera with these layers: the full art blooms most, the shape ronin's plain effects less, the cut
 * effects take the bloom with them.
 */
JAMGAME_API float Bloom(const FJamLayers& Layers, const FJamNeonSettings& Settings);

/** The light and the post process for the layers on show (T-077). */
JAMGAME_API EJamLight LightFor(const FJamLayers& Shown);

/** T-094: the film look of a tier: the full art only, the lower tiers stay clean. */
JAMGAME_API FJamFilmLook FilmLookFor(EJamLight Light, const FJamNeonSettings& Settings);
} // namespace JamNeonRules
