#include "Sim/JamAimRules.h"

bool JamAimRules::FromRay(const FVector& Origin, const FVector& Dir, float PlaneZ, const FVector2D& From, FJamAim& InOutAim)
{
	if (FMath::IsNearlyZero(Dir.Z))
	{
		return false;
	}
	const double Distance = (PlaneZ - Origin.Z) / Dir.Z;
	if (Distance <= 0.0)
	{
		return false;
	}
	const FVector Hit = Origin + Dir * Distance;
	InOutAim.Point = FVector2D(Hit.X, Hit.Y);
	const FVector2D ToPoint = InOutAim.Point - From;
	// Closer than a centimetre there is no direction to speak of.
	if (ToPoint.SizeSquared() > 1.0)
	{
		InOutAim.Dir = ToPoint.GetSafeNormal();
	}
	return true;
}

FVector2D JamAimRules::FacingTo(const FVector2D& Point, const FVector2D& From, const FVector2D& Keep)
{
	const FVector2D ToPoint = Point - From;
	// The same centimetre as FromRay: closer than that there is no direction to speak of.
	return ToPoint.SizeSquared() > 1.0 ? ToPoint.GetSafeNormal() : Keep;
}

FVector2D JamAimRules::FollowCamera(const FVector2D& Point, const FVector& CameraWas, const FVector& CameraNow)
{
	return Point + FVector2D(CameraNow.X - CameraWas.X, CameraNow.Y - CameraWas.Y);
}

FJamAim JamAimRules::FromYaw(float YawDeg, const FVector2D& From, float Reach)
{
	double Sin = 0.0;
	double Cos = 1.0;
	FMath::SinCos(&Sin, &Cos, FMath::DegreesToRadians(static_cast<double>(YawDeg)));
	FJamAim Aim;
	Aim.Dir = FVector2D(Cos, Sin);
	Aim.Point = From + Aim.Dir * Reach;
	return Aim;
}
