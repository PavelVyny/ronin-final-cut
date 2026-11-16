#pragma once

#include "CoreMinimal.h"

struct FJamBalanceData;
struct FJamLayers;

/** One segment of a neon bar, as fractions of the bar's length: it spans From to To and is lit from From to FillTo. */
struct FJamBarSegment
{
	float From = 0.f;
	float To = 0.f;
	/** From when the segment is empty (only its outline shows), To when it is full. */
	float FillTo = 0.f;

	bool IsLit() const { return FillTo > From; }
};

/** A slanted quad's corners: top left, top right, bottom right, bottom left. */
struct FJamSlantQuad
{
	FVector2D Corners[4];
};

/**
 * T-104: the layout of the full art's neon bars (Submission/hud_mock_neon.png): the fill cut into segments with a thin
 * gap, the cut marks on the Publisher's bar where he steals a feature, and the full Ultimate's pulse. Presentation only.
 * Tests: Jam.Hud.Neon*
 */
namespace JamNeonBarRules
{
/** How many steal points the Publisher's bar marks (the fourth, the final note's, sits at its very end). */
inline constexpr int32 CutMarkCount = 3;

/**
 * Count segments over the whole bar, each GapShare of the bar apart, lit up to Fill (0 to 1). The first starts at 0
 * and the last ends at 1; the segment Fill falls in is lit only up to it.
 */
JAMGAME_API TArray<FJamBarSegment> Segments(int32 Count, float GapShare, float Fill);

/** The shares of the Publisher's health at which he steals a feature, the first CutMarkCount of them, highest first. */
JAMGAME_API TArray<float> CutMarks(const FJamBalanceData& Balance);

/**
 * The parallelogram from X0 to X1 between rows Y0 and Y1, every point shifted right by (BottomY - y) * Slope: one slope
 * and one bottom line for everything on a bar (frame, plate, segments, cut marks, name tab), so their edges stay
 * parallel whatever their heights (the author's play-through: one pixel offset for shapes of different heights gave
 * different angles). X0 and X1 are where the edges cross BottomY.
 */
JAMGAME_API FJamSlantQuad Slanted(float X0, float Y0, float X1, float Y1, float BottomY, float Slope);

/** Whether the HUD draws the neon bars: they outlive the world's art by a cut, going with the hero's own look. */
JAMGAME_API bool Shows(const FJamLayers& Shown);

/** How much a full Ultimate bar glows towards white now (0 to 1, at Hz); 0 while the meter is not full. */
JAMGAME_API float UltimatePulse(float Meter, float Seconds, float Hz);
} // namespace JamNeonBarRules
