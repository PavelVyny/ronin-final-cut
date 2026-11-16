#pragma once

#include "CoreMinimal.h"

/** Where the arrow to an off-screen fighter stands on the screen. */
struct FJamEdgeMarker
{
	bool bShow = false;
	/** Screen pixels: on the screen edge, inset by the margin. */
	FVector2D Pos = FVector2D::ZeroVector;
	/** Where the arrow points, in degrees on the screen: 0 to the right, 90 down. */
	float Angle = 0.f;
};

/**
 * The arrow at the screen edge that shows where the Publisher is when the camera has lost him (T-045).
 * Tests: Jam.Hud.*
 */
namespace JamMarkerRules
{
/**
 * The marker for a point projected to Target on a screen of ViewSize pixels: hidden while the point is inside the
 * screen less Margin on the sides and the bottom and less TopInset at the top (the Publisher's health bar lives there),
 * otherwise on that inset edge where the line from the screen centre to the point crosses it.
 */
JAMGAME_API FJamEdgeMarker EdgeMarker(const FVector2D& Target, const FVector2D& ViewSize, float Margin, float TopInset);
} // namespace JamMarkerRules
