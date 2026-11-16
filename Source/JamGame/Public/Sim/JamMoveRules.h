#pragma once

#include "CoreMinimal.h"

/**
 * How a round body moves in the round arena.
 * Tests: Jam.Move.*
 */
namespace JamMoveRules
{
/** Move is a direction with length up to 1 (longer input is cut to 1, so a diagonal is not faster). */
JAMGAME_API FVector2D Velocity(const FVector2D& Move, float Speed);

/** Keeps a body of BodyRadius whole inside the arena. Pushing into the edge at an angle slides along it. */
JAMGAME_API FVector2D ClampToArena(const FVector2D& Pos, float BodyRadius, float ArenaRadius);

/** Moves a body out of another one along the line between them. The other body stays where it is. */
JAMGAME_API FVector2D PushOut(const FVector2D& Pos, float Radius, const FVector2D& OtherPos, float OtherRadius);

/**
 * Turns device input (X right, Y forward) into a world direction for a view that looks along BasisYawDeg.
 * Yaw 0 looks along +X with +Y to the right, as everywhere in the engine.
 */
JAMGAME_API FVector2D WorldMove(const FVector2D& Input, float BasisYawDeg);
} // namespace JamMoveRules
