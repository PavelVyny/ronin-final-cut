#include "UI/JamCardRules.h"

float JamCardRules::Pop(float Age, float Delay, float Seconds)
{
	if (Seconds <= 0.f)
	{
		return 1.f;
	}
	const float T = FMath::Clamp((Age - Delay) / Seconds, 0.f, 1.f);
	if (T <= 0.f || T >= 1.f)
	{
		return T;
	}
	// Ease-out-back: about 10% past the end before it settles.
	constexpr float Back = 1.70158f;
	const float U = T - 1.f;
	return 1.f + (Back + 1.f) * U * U * U + Back * U * U;
}

FBox2D JamCardRules::OptionBox(int32 Index, int32 Count, const FVector2D& Screen)
{
	const double Width = OptionWidth * Screen.X;
	const double Gap = OptionGap * Screen.X;
	const double Left = 0.5 * (Screen.X - Count * Width - (Count - 1) * Gap) + Index * (Width + Gap);
	const double Top = (CardTop + OptionRow) * Screen.Y;
	return FBox2D(FVector2D(Left, Top), FVector2D(Left + Width, Top + OptionHeight * Screen.Y));
}

int32 JamCardRules::OptionAt(const FVector2D& At, int32 Count, const FVector2D& Screen)
{
	for (int32 Index = 0; Index < Count; ++Index)
	{
		if (OptionBox(Index, Count, Screen).IsInsideOrOn(At))
		{
			return Index;
		}
	}
	return INDEX_NONE;
}
