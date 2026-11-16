#pragma once

#include "Core/JamBalance.h"
#include "Core/JamTypes.h"
#include "CoreMinimal.h"

/**
 * How the fight turns: a steal at a threshold, the loss, the stun at zero and the win. The Publisher does not die at
 * zero: he is stunned, everything he took goes back to the hero, and his Ultimate ends the fight.
 * Tests: Jam.Outcome.*, Jam.Fight.HeroDiesAtZeroHealth
 */
namespace JamOutcomeRules
{
/** The phase after a step, from what the step left behind. The hero's death comes first: a trade is a loss. */
JAMGAME_API EJamPhase Next(EJamPhase Phase, float HeroHealth, float BossHealth, float StealFloor, bool bFinalBlow);

/** Ends a step: moves the fight to its next phase and does what the change calls for. */
JAMGAME_API void Settle(FJamFight& Fight, const FJamBalanceData& Balance);
} // namespace JamOutcomeRules
