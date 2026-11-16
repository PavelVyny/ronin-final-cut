#pragma once

#include "Core/JamBalance.h"
#include "Core/JamTypes.h"
#include "CoreMinimal.h"

/**
 * The shuriken: a fast projectile on a short cooldown. Every few throws of the same fighter one is explosive: it goes
 * off where it hits or where its flight ends and hurts the thrower's enemy in a radius, never the thrower.
 * Tests: Jam.Shuriken.*, Jam.Fight.ExplosiveShurikenDamagesInRadius, Jam.Fight.CounterBelongsToTheThrower
 */
namespace JamShurikenRules
{
/** ThrowIndex counts a fighter's throws from 1. Every < 1 means no explosive throws. */
JAMGAME_API bool IsExplosive(int32 ThrowIndex, int32 Every);

/** True when the blast touches a round target. */
JAMGAME_API bool ExplosionHits(const FVector2D& Center, float Radius, const FVector2D& TargetPos, float TargetRadius);

/** The fighter holds the shuriken button: throws when the cooldown is over. */
JAMGAME_API void Throw(FJamFight& Fight, EJamSide Side, const FJamBalanceData& Balance);

/** An explosive shuriken goes off where it is. */
JAMGAME_API void Explode(FJamFight& Fight, const FJamProjectile& Shuriken, const FJamBalanceData& Balance);
} // namespace JamShurikenRules
