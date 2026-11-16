#pragma once

#include "Core/JamTypes.h"
#include "CoreMinimal.h"

/**
 * A flying shuriken lights the floor under it in its own colour (T-088): one small light per shot, a cap on how many
 * shine at once.
 * Tests: Jam.ShotLight.*
 */
namespace JamShotLightRules
{
/** Which of Projectiles light the floor, as indices: shurikens only, at most Cap, the newest (highest id) first. */
JAMGAME_API TArray<int32> Lit(TConstArrayView<FJamProjectile> Projectiles, int32 Cap);

/** A shot's light against a plain shuriken's: an explosive one ExplosiveBoost times as bright. */
JAMGAME_API float Brightness(const FJamProjectile& Projectile, float ExplosiveBoost);
} // namespace JamShotLightRules
