#pragma once

#include "Core/JamBalance.h"
#include "Core/JamTypes.h"
#include "CoreMinimal.h"

/**
 * The parry: a short window in which a projectile that reaches the fighter is not a hit but turns round and flies back
 * at the one who threw it, faster and harder.
 * Tests: Jam.Parry.*
 */
namespace JamParryRules
{
JAMGAME_API bool CanStart(const FJamParryState& State, int32 Tick);

/** Opens the window; with a raise (the Publisher's stance, T-038) it opens only after RaiseSeconds. */
JAMGAME_API FJamParryState Start(int32 Tick, const FJamBalanceData& Balance, float RaiseSeconds = 0.f);

JAMGAME_API bool IsActive(const FJamParryState& State, int32 Tick);

/** T-059: a parry that sent a shot back is ready again at once, so a click into a burst parries every shot of it. */
JAMGAME_API void Rearm(FJamParryState& State, int32 Tick);

/** True while the shield goes up: started, not yet sending anything back. */
JAMGAME_API bool IsRaising(const FJamParryState& State, int32 Tick);

/**
 * True when an enemy projectile would reach Side within one parry window if he stood still: then his strike (LMB)
 * parries instead of swinging.
 */
JAMGAME_API bool ShotInReach(const FJamFight& Fight, EJamSide Side, const FJamBalanceData& Balance);

/** True when an enemy projectile would reach Side within Seconds if he stood still. */
JAMGAME_API bool ShotWithin(const FJamFight& Fight, EJamSide Side, float Seconds);

/** Turns the projectile round: it belongs to the parrying side and flies at the former owner's position. */
JAMGAME_API void Deflect(FJamProjectile& Projectile, const FVector2D& FormerOwnerPos, int32 Tick, const FJamBalanceData& Balance);
} // namespace JamParryRules
