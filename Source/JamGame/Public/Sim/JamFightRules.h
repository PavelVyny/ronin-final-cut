#pragma once

#include "Core/JamBalance.h"
#include "Core/JamTypes.h"
#include "CoreMinimal.h"

/**
 * The fight as a pure function of its state, two intents and the balance numbers. No actors, no clocks, no globals: the
 * same seed and the same intents give the same fight, which is what the tests and the bot playthrough rely on.
 * Tests: Jam.Move.*, Jam.Aim.FighterFacesTheAim, Jam.Shot.*, Jam.Projectile.IgnoresItsOwner, Jam.Health.BallDamagesTheBoss,
 * Jam.Fight.*
 */
namespace JamFightRules
{
/** A fresh fight: the hero and the boss on their marks at full health. */
JAMGAME_API FJamFight Start(const FJamBalanceData& Balance, int32 Seed);

/** One fixed step of JamSim::StepDt seconds. Fight.Events holds what happened in it. */
JAMGAME_API void Step(FJamFight& Fight, const FJamIntents& Intents, const FJamBalanceData& Balance);

/** One step of the real game: the hero does what HeroIntent says, the Publisher decides for himself. */
JAMGAME_API void Play(FJamFight& Fight, const FJamIntent& HeroIntent, const FJamBalanceData& Balance);
} // namespace JamFightRules
