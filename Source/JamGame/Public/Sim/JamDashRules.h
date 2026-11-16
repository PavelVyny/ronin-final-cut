#pragma once

#include "Core/JamBalance.h"
#include "Core/JamTypes.h"
#include "CoreMinimal.h"

/**
 * The dash: a short burst along a fixed line with a window in which nothing hurts.
 * Tests: Jam.Dash.*, Jam.Fight.DashThroughAProjectileTakesNoDamage
 */
namespace JamDashRules
{
/** Where a dash goes: where the fighter runs, or where he faces when he stands. */
JAMGAME_API FVector2D Direction(const FVector2D& Move, const FVector2D& Facing);

/** The state of a dash that starts on this step. Its steps add up to the configured distance exactly. */
JAMGAME_API FJamDashState Start(const FVector2D& Dir, int32 Tick, const FJamBalanceData& Balance);

JAMGAME_API bool CanStart(const FJamDashState& State, int32 Tick);
JAMGAME_API bool IsDashing(const FJamDashState& State, int32 Tick);
JAMGAME_API bool IsInvulnerable(const FJamDashState& State, int32 Tick);
} // namespace JamDashRules
