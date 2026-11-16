#pragma once

#include "Core/JamBalance.h"
#include "Core/JamTypes.h"
#include "CoreMinimal.h"

/**
 * The Publisher's cuts. His health stops at each threshold; there he grabs two of the hero's mechanics and the player
 * gives one of them away on a card. While the card is up the fight stands still: nothing flies, nothing hurts, only the
 * card's clock runs. A choice card never offers the Katana. The last threshold is not a choice: he takes whatever is
 * left, the Katana with it.
 * Tests: Jam.Steal.*
 */
namespace JamStealRules
{
inline constexpr int32 ThresholdCount = 4;

/** Boss health at which the steal number Index waits; 0 when there is no such steal. */
JAMGAME_API float ThresholdHealth(int32 Index, const FJamBalanceData& Balance);

/** Health after a hit that may not cross the floor. */
JAMGAME_API float ClampToThreshold(float Health, float Floor);

/** What goes on the card: two of the hero's mechanics but the Katana, picked by the stream, or all of them with bTakesAll. */
JAMGAME_API TArray<EJamMechanic> Offer(const FJamLedger& Ledger, bool bTakesAll, FRandomStream& Stream);

/** The grab: clears the stage and puts the card up. A hero with nothing left is passed by without a card. */
JAMGAME_API void Open(FJamFight& Fight, const FJamBalanceData& Balance);

/** One step of the open card. An answer counts once the card is unlocked; when its time is up the left option goes. */
JAMGAME_API void Advance(FJamFight& Fight, int32 Choice, const FJamBalanceData& Balance);

/** Settles the steal: the picked option (or everything) goes to the boss and the fight goes on. */
JAMGAME_API void Resolve(FJamFight& Fight, int32 Choice, const FJamBalanceData& Balance);

/** The card's timer for the screen: 1 when it opens, 0 when the time is up. */
JAMGAME_API float TimeLeft(const FJamStealState& Steal, float Tick);
} // namespace JamStealRules
