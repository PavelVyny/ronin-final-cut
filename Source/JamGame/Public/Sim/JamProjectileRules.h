#pragma once

#include "Core/JamTypes.h"
#include "CoreMinimal.h"

/**
 * Flight of the projectiles: balls now, shurikens and memos later.
 * Tests: Jam.Projectile.*
 */
namespace JamProjectileRules
{
/**
 * True when a round projectile that moved from From to To in one step touched a circle. The whole path counts, so a fast
 * projectile does not jump over a thin target.
 */
JAMGAME_API bool SweepHitsCircle(const FVector2D& From, const FVector2D& To, float Radius, const FVector2D& Center, float CircleRadius);

/** True when the projectile left the arena or lived its time. */
JAMGAME_API bool IsGone(const FJamProjectile& Projectile, int32 Tick, float ArenaRadius);
} // namespace JamProjectileRules
