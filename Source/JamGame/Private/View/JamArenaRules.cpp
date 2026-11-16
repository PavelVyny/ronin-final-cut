#include "View/JamArenaRules.h"

EJamArenaLook JamArenaRules::LookFor(const FJamLayers& Layers)
{
	if (!Layers.bMaterials)
	{
		return EJamArenaLook::Greybox;
	}
	return Layers.bArt ? EJamArenaLook::Shrine : EJamArenaLook::BareShrine;
}

int32 JamArenaRules::StepSplashes(float& Carry, float Distance, float Stride)
{
	if (Stride <= 0.f)
	{
		Carry = 0.f;
		return 0;
	}
	Carry += FMath::Max(Distance, 0.f);
	const int32 Count = FMath::FloorToInt32(Carry / Stride);
	Carry -= Count * Stride;
	return Count;
}

FJamBackdropPlace JamArenaRules::PlaceBackdrop(
	const FVector& Eye, const FVector& LookAt, float Fov, float Aspect, float PictureAspect, double Distance)
{
	// The engine keeps the vertical field of view; Fov is the horizontal one on a 16:9 screen.
	constexpr double ReferenceAspect = 16.0 / 9.0;
	const double HalfTanY = FMath::Tan(FMath::DegreesToRadians(0.5 * Fov)) / ReferenceAspect;
	const double HalfHeight = Distance * HalfTanY;
	const double HalfWidth = HalfHeight * FMath::Max(Aspect, 0.01f);
	const double Ground = FVector2D(LookAt - Eye).Size();
	const double ViewCentreZ = Eye.Z + (Ground > 0.0 ? Distance * (LookAt.Z - Eye.Z) / Ground : 0.0);
	// Cover: the full height, and wider still when the screen is wider than the picture; the overflow goes below. A
	// sliver of overscan, so no hairline of the sky behind shows at an edge.
	constexpr double Overscan = 1.01;
	const double Picture = FMath::Max(PictureAspect, 0.01f);
	FJamBackdropPlace Place;
	Place.Height = Overscan * FMath::Max(2.0 * HalfHeight, 2.0 * HalfWidth / Picture);
	Place.Width = Place.Height * Picture;
	Place.CentreZ = ViewCentreZ + HalfHeight + 0.5 * (Overscan - 1.0) * Place.Height - 0.5 * Place.Height;
	return Place;
}
