#pragma once

#include "Core/JamBalance.h"
#include "Core/JamTypes.h"
#include "CoreMinimal.h"

/**
 * The heal the Publisher drops (T-049): once a drop is due (HealDropInterval after the last one, or after the start),
 * the next hit on him knocks a cash stack out of his pocket. It flies HealToss towards the hero, lands, and lies there
 * HealLife; the hero takes it by touch, if he is hurt. Never more than HealMaxOnFloor at once.
 * Tests: Jam.Heal.*
 */
namespace JamHealRules
{
/** Knocks a heal out on a hit of this step, when one is due and the floor has room. */
JAMGAME_API void Drop(FJamFight& Fight, const FJamBalanceData& Balance);

/** The hurt hero takes the landed heals he touches; the ones whose life is over are gone. */
JAMGAME_API void Collect(FJamFight& Fight, const FJamBalanceData& Balance);
} // namespace JamHealRules
