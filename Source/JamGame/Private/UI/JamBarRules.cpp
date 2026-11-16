#include "UI/JamBarRules.h"

void JamBarRules::Track(FJamBarState& Bar, float Fraction, float DeltaSeconds, const FJamBarSettings& Settings)
{
	Fraction = FMath::Clamp(Fraction, 0.f, 1.f);
	if (!Bar.bSet)
	{
		Bar.Fill = Bar.Chunk = Bar.Preview = Fraction;
		Bar.Calm = 0.f;
		Bar.bSet = true;
		return;
	}
	if (Fraction < Bar.Fill)
	{
		// A hit: the solid part drops at once, the white part keeps its top so it grows while the hits keep coming.
		Bar.Chunk = FMath::Max(Bar.Chunk, Bar.Fill);
		Bar.Fill = Fraction;
		Bar.Preview = Fraction;
		Bar.Calm = 0.f;
		return;
	}
	Bar.Preview = Fraction;
	Bar.Fill = FMath::Min(Bar.Preview, Bar.Fill + Settings.HealFill * DeltaSeconds);
	Bar.Calm += DeltaSeconds;
	if (Bar.Calm >= Settings.ChunkHold)
	{
		Bar.Chunk -= Settings.ChunkDrain * DeltaSeconds;
	}
	Bar.Chunk = FMath::Max(Bar.Chunk, Bar.Fill);
}
