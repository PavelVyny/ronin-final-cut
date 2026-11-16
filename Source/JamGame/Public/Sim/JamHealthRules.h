#pragma once

#include "CoreMinimal.h"

/**
 * Health arithmetic of a fighter.
 * Tests: Jam.Health.*
 */
namespace JamHealthRules
{
/** Health after a hit: never below zero, untouched while invulnerable, and damage never heals. */
JAMGAME_API float Apply(float Health, float Damage, bool bInvulnerable);

/**
 * How much harder the hero hits after StealsDone cuts: as listed before the first, by CutScale from it on (the author's
 * play-through: the fight was too hard after the first cut).
 */
JAMGAME_API float HeroDamageScale(int32 StealsDone, float CutScale);
} // namespace JamHealthRules
