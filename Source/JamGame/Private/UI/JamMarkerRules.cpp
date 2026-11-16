#include "UI/JamMarkerRules.h"

FJamEdgeMarker JamMarkerRules::EdgeMarker(const FVector2D& Target, const FVector2D& ViewSize, float Margin, float TopInset)
{
	FJamEdgeMarker Marker;
	const FVector2D Centre = 0.5 * ViewSize;
	// The box the arrow lives on: the screen less the margin, and less the inset at the top.
	const FVector2D Low(Margin, FMath::Min<double>(TopInset, Centre.Y - 1.0));
	const FVector2D High(ViewSize.X - Margin, ViewSize.Y - Margin);
	if (Target.X >= Low.X && Target.X <= High.X && Target.Y >= Low.Y && Target.Y <= High.Y)
	{
		return Marker;
	}
	// Walk from the centre towards the point until the first edge of the box: the arrow sits where the eye looks for him.
	const FVector2D Away = Target - Centre;
	const auto ScaleTo = [](double Along, double FromCentreLow, double FromCentreHigh)
	{
		if (FMath::Abs(Along) <= UE_KINDA_SMALL_NUMBER)
		{
			return static_cast<double>(UE_BIG_NUMBER);
		}
		return (Along > 0.0 ? FromCentreHigh : FromCentreLow) / Along;
	};
	const double ScaleX = ScaleTo(Away.X, Low.X - Centre.X, High.X - Centre.X);
	const double ScaleY = ScaleTo(Away.Y, Low.Y - Centre.Y, High.Y - Centre.Y);
	Marker.bShow = true;
	Marker.Pos = Centre + Away * FMath::Min(ScaleX, ScaleY);
	Marker.Angle = FMath::RadiansToDegrees(FMath::Atan2(Away.Y, Away.X));
	return Marker;
}
