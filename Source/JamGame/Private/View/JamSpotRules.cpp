#include "View/JamSpotRules.h"

FJamSpot JamSpotRules::Follow(const FJamSpot& Spot, const FVector2D& Target, float Dt, float Lag)
{
	if (Dt <= 0.f)
	{
		return Spot;
	}
	// A critically damped spring stepped exactly (Game Programming Gems 4, 1.10): its stiffness 2 / Lag makes it trail a
	// steady walk by Lag seconds, and the exponential's series keeps it stable at any Dt.
	const float Omega = 2.f / FMath::Max(Lag, UE_KINDA_SMALL_NUMBER);
	const float X = Omega * Dt;
	const float Decay = 1.f / (1.f + X + 0.48f * X * X + 0.235f * X * X * X);
	const FVector2D Off = Spot.At - Target;
	const FVector2D Pull = (Spot.Velocity + Omega * Off) * Dt;
	FJamSpot Next;
	Next.Velocity = (Spot.Velocity - Omega * Pull) * Decay;
	Next.At = Target + (Off + Pull) * Decay;
	return Next;
}

FVector2D JamSpotRules::Sway(float Time, float Reach)
{
	// Two slow waves on each axis at periods that never line up: a hand wanders, it does not loop. Each axis keeps
	// within 0.7 of Reach, so the two together stay inside it.
	const float X = 0.42f * FMath::Sin(0.9f * Time) + 0.28f * FMath::Sin(1.7f * Time + 1.3f);
	const float Y = 0.42f * FMath::Sin(0.7f * Time + 2.1f) + 0.28f * FMath::Sin(1.9f * Time + 0.4f);
	return FVector2D(X, Y) * Reach;
}

FVector2D JamSpotRules::SplitOff(const FVector2D& BossSpot, const FVector2D& Hero, float ToHero)
{
	return FMath::Lerp(BossSpot, Hero, static_cast<double>(FMath::Clamp(ToHero, 0.f, 1.f)));
}

float JamSpotRules::SplitShare(float ToHero)
{
	return FMath::Clamp(4.f * ToHero, 0.f, 1.f);
}
