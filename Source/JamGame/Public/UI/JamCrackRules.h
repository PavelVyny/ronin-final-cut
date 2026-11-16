#pragma once

#include "CoreMinimal.h"

/**
 * The crack between two looks of the game (T-068, slowed down in T-071): when a cut changes the layers, the old picture
 * is held as a frozen frame. A crack runs down it along a fault line that wanders across the screen, glows whole for a
 * moment, then the two halves slide apart and off the screen, showing the new look behind them. The fight holds while
 * it plays.
 * Tests: Jam.Crack.*
 */
namespace JamCrackRules
{
/** The crack runs from the top of the frame to the bottom. */
inline constexpr float RunSeconds = 0.5f;

/** It glows whole before the halves move: the eye finds the line. */
inline constexpr float HoldSeconds = 0.3f;

/** The halves slide apart and off the screen, slow at first so it is clear which went where. */
inline constexpr float PartSeconds = 1.3f;

/** How long the whole crack plays; the fight holds that long. */
inline constexpr float Seconds = RunSeconds + HoldSeconds + PartSeconds;

/** The fault line keeps this share of the width clear of each screen edge. */
inline constexpr float FaultMargin = 0.2f;

/** How far down the frame the crack has run Age seconds in, a share of the height, 0 to 1. */
JAMGAME_API float Reach(float Age);

/**
 * How far each half has slid Age seconds in, a share of the screen width: 0 until the crack has run and held, then
 * eased in and out up to 1 - FaultMargin, where both halves have left the screen.
 */
JAMGAME_API float Shift(float Age);

/** Whether the crack has played to the end and the new look is whole. */
JAMGAME_API bool IsOver(float Age, float Duration);

/**
 * Where the fault line crosses the height V (0 at the top, 1 at the bottom), a share of the screen width: a random
 * looking path that bends sharply a few times and zigzags finely between, within FaultMargin of the edges. The same for
 * the same V and seed (a crack keeps its shape while it plays), another path for another seed.
 */
JAMGAME_API float FaultX(float V, int32 Seed);
} // namespace JamCrackRules
