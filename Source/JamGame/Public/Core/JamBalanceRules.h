#pragma once

#include "Core/JamBalance.h"
#include "CoreMinimal.h"

/**
 * Sanity checks of the balance numbers, run at the start of a fight and by the test of the shipped ini.
 * Tests: Jam.Balance.*
 */
namespace JamBalanceRules
{
/** Human-readable problems of the numbers, empty when they are fine. Every problem names its field. */
JAMGAME_API TArray<FString> Problems(const FJamBalanceData& Balance);
} // namespace JamBalanceRules
