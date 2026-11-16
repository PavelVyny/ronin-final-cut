#pragma once

#include "CoreMinimal.h"

/**
 * The steal card on the screen (T-094): where its options are, so a click can pick one, and how a panel pops in, with a
 * slight overshoot before it settles (ease-out-back), the card's choices a beat after it.
 * Tests: Jam.Card.*
 */
namespace JamCardRules
{
/** The card's band as fractions of the screen's height. */
inline constexpr float CardTop = 0.24f;
inline constexpr float CardHeight = 0.46f;
/** The options' row, from the band's top, and their size and gap, as fractions of the screen. */
inline constexpr float OptionRow = 0.16f;
inline constexpr float OptionWidth = 0.2f;
inline constexpr float OptionHeight = 0.14f;
inline constexpr float OptionGap = 0.03f;
/** A panel pops in over this long; the card's options start this long after the card. */
inline constexpr float PopSeconds = 0.3f;
inline constexpr float OptionDelay = 0.12f;

/** How far a panel has popped in, Age seconds after it opened and Delay before it starts: 0, past 1, then 1. */
JAMGAME_API float Pop(float Age, float Delay = 0.f, float Seconds = PopSeconds);

/** Option Index of Count side by side, centred on a screen of Screen pixels. */
JAMGAME_API FBox2D OptionBox(int32 Index, int32 Count, const FVector2D& Screen);

/** The option of Count at At, INDEX_NONE between and around them. */
JAMGAME_API int32 OptionAt(const FVector2D& At, int32 Count, const FVector2D& Screen);
} // namespace JamCardRules
