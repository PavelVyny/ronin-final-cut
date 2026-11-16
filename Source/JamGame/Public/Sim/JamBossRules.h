#pragma once

#include "Core/JamBalance.h"
#include "Core/JamTypes.h"
#include "CoreMinimal.h"

/**
 * The Publisher: his brain and his own two attacks. The brain writes the same intent a player writes; it chases the
 * hero, slams when he is close, throws a fan of memos when he is not, and stands still for a moment after each attack:
 * that pause is the player's opening. What he stole he uses through the hero's own buttons: the katana up close, the
 * shuriken from afar, the dash to close in or to leave a strike.
 * Tests: Jam.Boss.*
 */
namespace JamBossRules
{
/** What the boss wants to do on this step. */
JAMGAME_API FJamIntent Decide(const FJamFight& Fight, const FJamBalanceData& Balance);

/** True while the boss stands: an attack in progress, the pause after it, or the stun at the end. */
JAMGAME_API bool IsBusy(const FJamFight& Fight);

/**
 * The direction in which a projectile thrown from From at Speed meets a target that keeps its course. HeadStart is how
 * far in front of the thrower the projectile appears. A target that cannot be caught is aimed at directly.
 */
JAMGAME_API FVector2D LeadDirection(
	const FVector2D& From, const FVector2D& TargetPos, const FVector2D& TargetVelocity, float Speed, float HeadStart);

/** Directions of a volley: an even fan of Count unit vectors, SpreadDeg wide, around the aim. */
JAMGAME_API TArray<FVector2D> VolleyDirections(const FVector2D& AimDir, int32 Count, float SpreadDeg);

/** Starts the attack the intent asks for, when the boss is free and the attack is ready. */
JAMGAME_API void Act(FJamFight& Fight, const FJamIntent& Intent, const FJamBalanceData& Balance);
} // namespace JamBossRules
