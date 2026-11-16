#pragma once

#include "CoreMinimal.h"

/** Where the key light's spot is on the floor and how fast it is moving there, in cm and cm/s. */
struct FJamSpot
{
	FVector2D At = FVector2D::ZeroVector;
	FVector2D Velocity = FVector2D::ZeroVector;
};

/**
 * The key light on the Publisher (T-087) is worked by a hand, not glued to him: the spot trails him on a critically
 * damped spring, so it eases after him and never swings past, and it sways a little as an operator's would.
 * Tests: Jam.Spot.*
 */
namespace JamSpotRules
{
/** The spot Dt seconds on, chasing Target; once it has his pace it trails him by Lag seconds. Stable at any Dt. */
JAMGAME_API FJamSpot Follow(const FJamSpot& Spot, const FVector2D& Target, float Dt, float Lag);

/** The hand's sway at Time: a slow wander of at most Reach cm, smooth from frame to frame. */
JAMGAME_API FVector2D Sway(float Time, float Reach);

/**
 * T-100: the hero's own spot while the intro's camera goes over from the Publisher to him, ToHero of the way
 * (JamIntroRules::CameraToHero): it starts on the Publisher's spot, travels with the camera and lands on the hero as he
 * comes into frame.
 */
JAMGAME_API FVector2D SplitOff(const FVector2D& BossSpot, const FVector2D& Hero, float ToHero);

/** How much of its light the hero's spot has ToHero of the way over: none inside the Publisher's beam, all of it by a quarter. */
JAMGAME_API float SplitShare(float ToHero);
} // namespace JamSpotRules
