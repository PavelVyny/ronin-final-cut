#pragma once

#include "Core/JamBalance.h"
#include "Core/JamTypes.h"
#include "CoreMinimal.h"
#include "Misc/Optional.h"

/**
 * The Publisher's slam grows with each cut (T-050): one circle on the hero before the first cut, then a pattern that
 * also closes the floor around him and the hero's ways out. The circles land one after another, from him outwards.
 * There is always a spot the hero can walk to in time, and a cash stack on the floor is never covered.
 * Tests: Jam.Slam.*
 */
namespace JamSlamRules
{
/** How many circles one slam puts down after CutsDone cuts. */
JAMGAME_API int32 CircleCount(int32 CutsDone, const FJamBalanceData& Balance);

/**
 * The circles of one slam put down on Tick, in landing order. A single circle is the one on the hero. Three: the one
 * around the boss, the one on the hero, the cutter of his way back. Five (T-052): two at the boss's flanks, the one on
 * the hero, two wide behind him on either side of the way back, with SlamGap between the edges of any two (a circle
 * that finds no such room is left out). Ids are left to the caller. A cutter that would leave no safe spot is left
 * out; a circle over one of the Pickups is pushed off it.
 */
JAMGAME_API TArray<FJamHazard> Pattern(const FVector2D& BossPos, const FVector2D& HeroPos, int32 Count, const TArray<FJamPickup>& Pickups,
	int32 Tick, FRandomStream& Stream, const FJamBalanceData& Balance);

/**
 * The nearest spot that a fighter at Pos, walking straight at the hero's speed after ReactionSeconds, reaches without
 * being inside any of the Hazards on the step it lands. Unset when there is none.
 */
JAMGAME_API TOptional<FVector2D> SafeSpot(
	const TArray<FJamHazard>& Hazards, const FVector2D& Pos, int32 Tick, float ReactionSeconds, const FJamBalanceData& Balance);
} // namespace JamSlamRules
