#include "UI/JamCrackRules.h"

namespace
{
// The fault is cut by midpoint displacement: the line between its two ends is split in two again and again, each
// new point pushed sideways by less than the last, so it bends at every scale like a real crack.
constexpr int32 FaultLevels = 6;
constexpr int32 FaultPoints = 1 << FaultLevels;
// The first split always bends this far at least (a crack is never straight), up to the extra on top.
constexpr float FirstBendLeast = 0.07f;
constexpr float FirstBendExtra = 0.06f;
// The next split's push, and how much each further split keeps of it.
constexpr float NextBend = 0.17f;
constexpr float Roughness = 0.6f;

/** A hash in 0..1, not a random stream: the fault keeps its shape every frame and needs no state. */
float CrackHash(int32 Index, int32 Seed)
{
	const uint32 Hash = HashCombineFast(GetTypeHash(Index), GetTypeHash(Seed * 7919 + 13));
	return static_cast<float>(Hash % 10007u) / 10006.f;
}

void CutFault(int32 Seed, float (&Points)[FaultPoints + 1])
{
	Points[0] = 0.4f + 0.2f * CrackHash(0, Seed);
	Points[FaultPoints] = 0.4f + 0.2f * CrackHash(1, Seed);
	float Bend = NextBend;
	for (int32 Step = FaultPoints; Step > 1; Step /= 2)
	{
		for (int32 Index = Step / 2; Index < FaultPoints; Index += Step)
		{
			const float Hash = CrackHash(Index + Step * 100, Seed);
			const float Side = Hash < 0.5f ? -1.f : 1.f;
			const float Push =
				Step == FaultPoints ? Side * (FirstBendLeast + FirstBendExtra * CrackHash(2, Seed)) : (2.f * Hash - 1.f) * 0.5f * Bend;
			Points[Index] = 0.5f * (Points[Index - Step / 2] + Points[Index + Step / 2]) + Push;
		}
		if (Step < FaultPoints)
		{
			Bend *= Roughness;
		}
	}
}
} // namespace

float JamCrackRules::Reach(float Age)
{
	// An ease-out: the crack shoots off and slows as it reaches the bottom.
	const float T = FMath::Clamp(Age / RunSeconds, 0.f, 1.f);
	return T * (2.f - T);
}

float JamCrackRules::Shift(float Age)
{
	// Smoothstep: the halves come loose slowly, then slide away and off.
	const float T = FMath::Clamp((Age - RunSeconds - HoldSeconds) / PartSeconds, 0.f, 1.f);
	return (1.f - FaultMargin) * T * T * (3.f - 2.f * T);
}

bool JamCrackRules::IsOver(float Age, float Duration)
{
	return Age >= Duration;
}

float JamCrackRules::FaultX(float V, int32 Seed)
{
	float Points[FaultPoints + 1];
	CutFault(Seed, Points);
	const float At = FMath::Clamp(V, 0.f, 1.f) * FaultPoints;
	const int32 Index = FMath::Min(FMath::FloorToInt32(At), FaultPoints - 1);
	const float X = FMath::Lerp(Points[Index], Points[Index + 1], At - Index);
	return FMath::Clamp(X, FaultMargin, 1.f - FaultMargin);
}
