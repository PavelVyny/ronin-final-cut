#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"

/**
 * T-096: the HOW TO PLAY screen, a neon schematic of a keyboard and a mouse with only the fight's keys lit and thin
 * leader lines from each lit key to its label. Shown once after the opening comic, before the intro, with a countdown;
 * F1 brings it back over the fight, which waits under it. The keys come from JamKeys, the very bindings the controller
 * reads.
 *
 * Tests: Jam.HowTo.*
 */

/** Where a label sits: in the row over the keyboard, the row under it, or the column right of the mouse. */
enum class EJamHowToSide : uint8
{
	Above,
	Below,
	Right
};

/** One label of the schematic: the keys it points at, their caps as the label shows them, what they do. */
struct FJamHowToCallout
{
	TArray<FKey> Keys;
	FString Caps;
	FString Label;
	EJamHowToSide Side = EJamHowToSide::Above;
};

/** One key of the drawn keyboard or mouse: its cap, its box on the screen and whether a callout lights it. */
struct FJamHowToKey
{
	FKey Key;
	FString Cap;
	FBox2D Box = FBox2D(ForceInit);
	bool bLit = false;
};

/** The schematic placed on a screen: every key and mouse part, each label's box, and the leader lines. */
struct FJamHowToLayout
{
	TArray<FJamHowToKey> Keys;
	/** The keyboard's outline and the mouse's body. */
	FBox2D Board = FBox2D(ForceInit);
	FBox2D Mouse = FBox2D(ForceInit);
	/** One box per callout, in the callouts' order. */
	TArray<FBox2D> Labels;
	/** Per callout, one line per key: from the key's edge to the label. */
	TArray<TArray<TPair<FVector2D, FVector2D>>> Lines;
};

namespace JamHowToRules
{
/** The screen after the comic starts the fight on its own after this long. */
inline constexpr float AutoStartSeconds = 15.f;
/** A key counts only this long after the screen came up: the click that ended the comic does not skip it too. */
inline constexpr float KeyGraceSeconds = 0.6f;

/** A key's cap as the screen prints it: "W", "SPACE", "F1", "LEFT CLICK". */
JAMGAME_API FString CapOf(const FKey& Key);

/** 2nd, 3rd, 4th, 11th, 21st. */
JAMGAME_API FString Ordinal(int32 Number);

/** The labels of the screen, from JamKeys; every ExplosiveEvery-th shuriken explodes (the balance's number). */
JAMGAME_API TArray<FJamHowToCallout> Callouts(int32 ExplosiveEvery);

/** Whole seconds left on the countdown, Age seconds after the screen came up; 0 once it has run out. */
JAMGAME_API int32 SecondsLeft(float Age);

/** Whether the screen goes: a key after the grace, or the countdown run out when there is one (not over the fight). */
JAMGAME_API bool IsOver(float Age, bool bAnyKey, bool bCountdown);

/** The schematic on a Screen, each label LabelSizes[i] big (measured by the HUD), in Callouts' order. */
JAMGAME_API FJamHowToLayout Layout(const FVector2D& Screen, const TArray<FJamHowToCallout>& Callouts, const TArray<FVector2D>& LabelSizes);
} // namespace JamHowToRules
