#pragma once

#include "Core/JamBalance.h"
#include "CoreMinimal.h"

/** The levels the menu's DIFFICULTY row cycles through (T-101), in its order. */
enum class EJamDifficulty : uint8
{
	Easy,
	Normal,
	Hard
};

/** What a level scales: the hero's health, every hit of his, the cash stack's heal and every hit of the Publisher's. */
struct FJamDifficultyScales
{
	float HeroHealth = 1.f;
	float HeroDamage = 1.f;
	float Heal = 1.f;
	float BossDamage = 1.f;
};

/**
 * T-101: difficulty adds no mechanic; it scales numbers the balance already has, by the level's block of [JamBalance].
 * NORMAL's block is all ones: it plays as the game did before the levels.
 * Tests: Jam.Difficulty.*
 */
namespace JamDifficultyRules
{
inline constexpr int32 Count = 3;

/** The level of the menu's index 0 EASY, 1 NORMAL, 2 HARD; out of range is NORMAL. */
JAMGAME_API EJamDifficulty FromIndex(int32 Index);

/** The level's block of scales in Balance. */
JAMGAME_API FJamDifficultyScales ScalesOf(const FJamBalanceData& Balance, EJamDifficulty Difficulty);

/** The balance a fight on Difficulty plays with: Balance with the level's scales applied. */
JAMGAME_API FJamBalanceData Apply(const FJamBalanceData& Balance, EJamDifficulty Difficulty);
} // namespace JamDifficultyRules
