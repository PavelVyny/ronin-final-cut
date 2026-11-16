#pragma once

#include "Core/JamBalance.h"
#include "Core/JamTypes.h"
#include "CoreMinimal.h"

/** The buttons of a fighter that a mechanic can sit on. */
enum class EJamButton : uint8
{
	Primary,
	Secondary,
	Dash,
	Parry,
	Ultimate
};

/** What a button does this very moment. */
enum class EJamAction : uint8
{
	None,
	Ball,
	Katana,
	Shuriken,
	Dash,
	Parry,
	Ultimate
};

/**
 * Who can do what. Every mechanic has one owner; the hero's attack buttons fall back to the weak ball when their
 * mechanic is gone, the boss gets nothing he was not given.
 * Tests: Jam.Loadout.*
 */
namespace JamLoadoutRules
{
/** The action behind a button for a side with this ledger. */
JAMGAME_API EJamAction ActionFor(EJamButton Button, EJamSide Side, const FJamLedger& Ledger);

/** Moves one mechanic to a new owner. */
JAMGAME_API void Give(FJamLedger& Ledger, EJamMechanic Mechanic, EJamSide NewOwner);

/** The first mechanic the side owns, in the order of the enum. False when it owns none. */
JAMGAME_API bool FirstOwned(const FJamLedger& Ledger, EJamSide Side, EJamMechanic& OutMechanic);

/** Everything the side owns, in the order of the enum. */
JAMGAME_API TArray<EJamMechanic> Owned(const FJamLedger& Ledger, EJamSide Side);

/**
 * The balance numbers as a side holds them. The hero's are the numbers themselves; in the Publisher's hands a stolen
 * mechanic is weaker and slower, and his katana warns before it lands: the player must be able to read what he made.
 */
JAMGAME_API FJamBalanceData InHandsOf(EJamSide Side, const FJamBalanceData& Balance);

/** Name of a mechanic for the screen. */
JAMGAME_API const TCHAR* Name(EJamMechanic Mechanic);
} // namespace JamLoadoutRules
