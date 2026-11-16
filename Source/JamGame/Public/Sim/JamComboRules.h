#pragma once

#include "Core/JamBalance.h"
#include "Core/JamTypes.h"
#include "CoreMinimal.h"

/**
 * The Publisher's signature chains (T-056): from the first cut he strings his attacks together, and each mechanic he
 * stole adds a chain of its own, so the order of the cuts changes how he fights. Every chain starts with a tell (he
 * stands and glows) and keeps a gap between its links; it ends with his usual pause.
 * Tests: Jam.Combo.*
 */
namespace JamComboRules
{
/** The chains he can start right now: what he owns, where the hero stands, what is off cooldown. None before a cut. */
JAMGAME_API TArray<EJamCombo> Available(const FJamFight& Fight, const FJamBalanceData& Balance);

/** One of Options from the stream, never Last when there is another. */
JAMGAME_API EJamCombo Pick(const TArray<EJamCombo>& Options, EJamCombo Last, FRandomStream& Stream);

/** The links of a chain that starts on this step, in order, the last one End. */
JAMGAME_API TArray<FJamComboLink> Plan(EJamCombo Combo, const FJamFight& Fight, const FJamBalanceData& Balance);

JAMGAME_API bool IsRunning(const FJamFight& Fight);

/** The link that goes on this step, or null. */
JAMGAME_API const FJamComboLink* LinkNow(const FJamFight& Fight);

/** True from the rush link of a running chain until its end. */
JAMGAME_API bool IsRushing(const FJamFight& Fight);

/** Where the pocket fan flies: at the nearest spot the hero can reach clear of the circles, or at him when none. */
JAMGAME_API FVector2D PocketAim(const FJamFight& Fight, const FJamBalanceData& Balance);
} // namespace JamComboRules
