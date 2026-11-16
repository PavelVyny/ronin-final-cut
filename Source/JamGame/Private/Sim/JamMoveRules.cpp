#include "Sim/JamMoveRules.h"

FVector2D JamMoveRules::Velocity(const FVector2D& Move, float Speed)
{
	const double Length = Move.Size();
	if (Length <= UE_SMALL_NUMBER)
	{
		return FVector2D::ZeroVector;
	}
	return Move * (FMath::Min(Length, 1.0) / Length * Speed);
}

FVector2D JamMoveRules::ClampToArena(const FVector2D& Pos, float BodyRadius, float ArenaRadius)
{
	const double Limit = FMath::Max(0.f, ArenaRadius - BodyRadius);
	const double Distance = Pos.Size();
	if (Distance <= Limit)
	{
		return Pos;
	}
	return Pos * (Limit / Distance);
}

FVector2D JamMoveRules::PushOut(const FVector2D& Pos, float Radius, const FVector2D& OtherPos, float OtherRadius)
{
	const double Gap = Radius + OtherRadius;
	const FVector2D Away = Pos - OtherPos;
	const double Distance = Away.Size();
	if (Distance >= Gap)
	{
		return Pos;
	}
	// Exactly on top of each other there is no line between them: any direction will do.
	const FVector2D Dir = Distance > UE_SMALL_NUMBER ? Away / Distance : FVector2D(1.0, 0.0);
	return OtherPos + Dir * Gap;
}

FVector2D JamMoveRules::WorldMove(const FVector2D& Input, float BasisYawDeg)
{
	double Sin = 0.0;
	double Cos = 1.0;
	FMath::SinCos(&Sin, &Cos, FMath::DegreesToRadians(static_cast<double>(BasisYawDeg)));
	const FVector2D Forward(Cos, Sin);
	const FVector2D Right(-Sin, Cos);
	return Forward * Input.Y + Right * Input.X;
}
