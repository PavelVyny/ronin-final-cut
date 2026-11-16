#pragma once

#include "CoreMinimal.h"

#include "JamBarRules.generated.h"

/**
 * Numbers of the health bars' delayed chunk and heal preview. The values live in the [JamBar] section of
 * Config/DefaultGame.ini, one line per field.
 */
USTRUCT()
struct JAMGAME_API FJamBarSettings
{
	GENERATED_BODY()

	/** Seconds without new damage before the white chunk starts to drain. */
	UPROPERTY()
	float ChunkHold = 0.f;

	/** How fast the white chunk drains, in whole bars per second. */
	UPROPERTY()
	float ChunkDrain = 0.f;

	/** How fast the bar fills up to a heal, in whole bars per second. */
	UPROPERTY()
	float HealFill = 0.f;
};

/** What one health bar shows, as fractions of the whole bar. Fill <= Chunk and Fill <= Preview. */
struct FJamBarState
{
	/** The solid part: the health the bar shows now. */
	float Fill = 1.f;
	/** The white part from Fill up to here: health just lost, waiting to drain. */
	float Chunk = 1.f;
	/** The light part from Fill up to here: health just gained, the bar fills up to it. */
	float Preview = 1.f;
	/** Seconds since the last damage. */
	float Calm = 0.f;
	/** False until the bar has seen the health once: the first look snaps to it. */
	bool bSet = false;
};

/**
 * The health bars of both fighters (T-053, the "delayed damage chunk" pattern): a hit drops the solid part at once and
 * leaves the lost part white; while hits keep coming the white part grows; after a pause with no damage it drains
 * away. A heal shows the gained part at once as a light preview and the solid part fills up to it. Presentation only:
 * the simulation never asks.
 * Tests: Jam.Hud.*
 */
namespace JamBarRules
{
inline const TCHAR* const Section = TEXT("JamBar");

/** Moves the bar DeltaSeconds on towards the health Fraction (0 to 1). */
JAMGAME_API void Track(FJamBarState& Bar, float Fraction, float DeltaSeconds, const FJamBarSettings& Settings);
} // namespace JamBarRules
