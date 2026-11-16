#pragma once

#include "Core/JamBalance.h"
#include "Core/JamTypes.h"
#include "CoreMinimal.h"

/**
 * The retry point. A death does not send the player to the start: the fight comes back as it stood after the last
 * steal, with the boss's health, the ledger and the hero's health of that moment (RetryHealth), both fighters on their marks.
 * Tests: Jam.Retry.*
 */
namespace JamCheckpointRules
{
/** What a retry starts from, taken from the fight as it stands now. */
JAMGAME_API FJamFight Capture(const FJamFight& Fight, const FJamBalanceData& Balance);

/**
 * The hero's health on a retry from a cut made while he had HealthAtTheCut: the same, except at RetryLowHealthShare of
 * his full health or less, when he comes back with RetryLowHealthRestore of it.
 */
JAMGAME_API float RetryHealth(float HealthAtTheCut, const FJamBalanceData& Balance);

/** A fight that starts from the saved point with new luck. */
JAMGAME_API FJamFight Restore(const FJamFight& Saved, int32 Seed);
} // namespace JamCheckpointRules
