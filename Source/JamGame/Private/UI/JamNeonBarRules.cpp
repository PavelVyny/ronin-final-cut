#include "UI/JamNeonBarRules.h"

#include "Core/JamBalance.h"
#include "Sim/JamLayerRules.h"
#include "Sim/JamStealRules.h"

TArray<FJamBarSegment> JamNeonBarRules::Segments(int32 Count, float GapShare, float Fill)
{
	TArray<FJamBarSegment> Out;
	if (Count <= 0)
	{
		return Out;
	}
	const float Width = (1.f - GapShare * (Count - 1)) / Count;
	const float Clamped = FMath::Clamp(Fill, 0.f, 1.f);
	Out.Reserve(Count);
	for (int32 Index = 0; Index < Count; ++Index)
	{
		FJamBarSegment Segment;
		Segment.From = Index * (Width + GapShare);
		// The last one ends exactly at the bar's end, whatever the float sum left.
		Segment.To = Index == Count - 1 ? 1.f : Segment.From + Width;
		Segment.FillTo = Clamped > Segment.From ? FMath::Min(Segment.To, Clamped) : Segment.From;
		Out.Add(Segment);
	}
	return Out;
}

TArray<float> JamNeonBarRules::CutMarks(const FJamBalanceData& Balance)
{
	TArray<float> Out;
	const float Max = FMath::Max(1.f, Balance.BossHealth);
	for (int32 Index = 0; Index < CutMarkCount; ++Index)
	{
		Out.Add(JamStealRules::ThresholdHealth(Index, Balance) / Max);
	}
	return Out;
}

FJamSlantQuad JamNeonBarRules::Slanted(float X0, float Y0, float X1, float Y1, float BottomY, float Slope)
{
	const float TopShift = (BottomY - Y0) * Slope;
	const float BottomShift = (BottomY - Y1) * Slope;
	FJamSlantQuad Quad;
	Quad.Corners[0] = FVector2D(X0 + TopShift, Y0);
	Quad.Corners[1] = FVector2D(X1 + TopShift, Y0);
	Quad.Corners[2] = FVector2D(X1 + BottomShift, Y1);
	Quad.Corners[3] = FVector2D(X0 + BottomShift, Y1);
	return Quad;
}

bool JamNeonBarRules::Shows(const FJamLayers& Shown)
{
	return Shown.bHeroArt;
}

float JamNeonBarRules::UltimatePulse(float Meter, float Seconds, float Hz)
{
	if (Meter < 1.f)
	{
		return 0.f;
	}
	return 0.5f + 0.5f * FMath::Sin(UE_TWO_PI * Hz * Seconds);
}
