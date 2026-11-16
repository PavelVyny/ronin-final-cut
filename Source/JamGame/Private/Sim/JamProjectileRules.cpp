#include "Sim/JamProjectileRules.h"

bool JamProjectileRules::SweepHitsCircle(
	const FVector2D& From, const FVector2D& To, float Radius, const FVector2D& Center, float CircleRadius)
{
	const FVector2D Closest = FMath::ClosestPointOnSegment2D(Center, From, To);
	const double Reach = Radius + CircleRadius;
	return FVector2D::DistSquared(Closest, Center) <= Reach * Reach;
}

bool JamProjectileRules::IsGone(const FJamProjectile& Projectile, int32 Tick, float ArenaRadius)
{
	return Tick >= Projectile.DieTick || Projectile.Pos.SizeSquared() > FMath::Square(static_cast<double>(ArenaRadius));
}
