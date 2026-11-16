#pragma once

#include "CoreMinimal.h"

/**
 * Loads a struct of numbers from a section of Config/DefaultGame.ini: one line per UPROPERTY, named as the field.
 * Tests: Jam.Balance.IniIsCompleteAndValid, Jam.Camera.IniIsComplete
 */
namespace JamConfig
{
/** Section with the balance numbers (FJamBalanceData). */
inline const TCHAR* const BalanceSection = TEXT("JamBalance");

/** Fills Data, a struct of type Struct. Fields without a readable line keep their value and are named in OutMissing. */
JAMGAME_API void LoadStruct(const UScriptStruct& Struct, void* Data, const TCHAR* Section, TArray<FString>* OutMissing);

template <typename T>
T Load(const TCHAR* Section, TArray<FString>* OutMissing = nullptr)
{
	T Value;
	LoadStruct(*T::StaticStruct(), &Value, Section, OutMissing);
	return Value;
}
} // namespace JamConfig
