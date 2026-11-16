#pragma once

#include "Core/JamTypes.h"
#include "CoreMinimal.h"

/**
 * Warning circles: a place that is shown first and hurts once, a moment later. A fighter who left it is safe.
 * Tests: Jam.Boss.SlamHurtsOnlyAfterTheTelegraph, Jam.Boss.SlamMissesAHeroWhoLeftTheCircle
 */
namespace JamHazardRules
{
/** True on the step the circle hits. */
JAMGAME_API bool IsDue(const FJamHazard& Hazard, int32 Tick);

/** True for a fighter whose centre is inside the circle. */
JAMGAME_API bool Hits(const FJamHazard& Hazard, const FVector2D& TargetPos);

/** How full the warning is: 0 when it appears, 1 when it hits. Tick may be a fraction, for the view. */
JAMGAME_API float Progress(const FJamHazard& Hazard, float Tick);

/** Lands the circles that are due on this step. */
JAMGAME_API void Advance(FJamFight& Fight);
} // namespace JamHazardRules
