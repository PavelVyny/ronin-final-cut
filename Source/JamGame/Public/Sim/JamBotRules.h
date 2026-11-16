#pragma once

#include "Core/JamBalance.h"
#include "Core/JamTypes.h"
#include "CoreMinimal.h"

/** Which option of the steal card the bot gives away. */
enum class EJamStealPolicy : uint8
{
	Left,
	Right,
	/** Left on one card, right on the next. */
	Mixed
};

/** How a whole fight played by the bot went. */
struct FJamBotRun
{
	/** Won or HeroDead; Fight when the time ran out first. */
	EJamPhase Outcome = EJamPhase::Fight;
	float Seconds = 0.f;
	float LowestHeroHealth = 0.f;
	/** What the boss had left at the end: where a lost fight was lost. */
	float BossHealthLeft = 0.f;
	/** The mechanics in the order the Publisher got them. */
	TArray<EJamMechanic> Stolen;
};

/**
 * A bot that plays a fighter through the same intent a person writes. It drives the hero in clips and in full-fight
 * tests, so a fight can be watched and checked without hands on the keyboard.
 * Tests: Jam.Bot.*
 */
namespace JamBotRules
{
/** Circles the opponent at Range and keeps shooting at it. */
JAMGAME_API FJamIntent Kite(const FJamFight& Fight, EJamSide Side, float Range);

/**
 * Plays the hero against the Publisher with whatever he still owns: keeps out of the slam range and throws from there,
 * runs in with the katana while the boss stands after an attack (only while it has the dash to get out again), leaves
 * the warning circles, walks out of the reach of a blade that is on its way and dashes through the memos that are about
 * to hit. Its circle turns the other way when the course ahead runs into a projectile or a blast and the way back does
 * not, and it keeps room between itself and the arena wall.
 */
JAMGAME_API FJamIntent Play(const FJamFight& Fight, EJamSide Side, const FJamBalanceData& Balance);

/** The bot's answer to the steal card: an option index once the card takes answers, -1 otherwise. */
JAMGAME_API int32 Choose(const FJamFight& Fight, EJamStealPolicy Policy);

/** A whole fight without a screen: the bot against the Publisher from the first step to a title, one life, no retry. */
JAMGAME_API FJamBotRun RunFight(const FJamBalanceData& Balance, int32 Seed, EJamStealPolicy Policy, float MaxSeconds);
} // namespace JamBotRules
