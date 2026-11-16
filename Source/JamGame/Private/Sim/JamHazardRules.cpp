#include "Sim/JamHazardRules.h"

#include "Sim/JamFightOps.h"

bool JamHazardRules::IsDue(const FJamHazard& Hazard, int32 Tick)
{
	return Tick >= Hazard.HitTick;
}

bool JamHazardRules::Hits(const FJamHazard& Hazard, const FVector2D& TargetPos)
{
	return FVector2D::DistSquared(Hazard.Pos, TargetPos) <= FMath::Square(static_cast<double>(Hazard.Radius));
}

float JamHazardRules::Progress(const FJamHazard& Hazard, float Tick)
{
	const float Length = static_cast<float>(FMath::Max(1, Hazard.HitTick - Hazard.StartTick));
	return FMath::Clamp((Tick - Hazard.StartTick) / Length, 0.f, 1.f);
}

void JamHazardRules::Advance(FJamFight& Fight)
{
	for (int32 Index = Fight.Hazards.Num() - 1; Index >= 0; --Index)
	{
		if (!IsDue(Fight.Hazards[Index], Fight.Tick))
		{
			continue;
		}
		const FJamHazard Hazard = Fight.Hazards[Index];
		Fight.Hazards.RemoveAtSwap(Index);
		JamFightOps::AddEvent(Fight, EJamEvent::Slam, Hazard.Owner, Hazard.Id, Hazard.Pos, Hazard.Radius);
		const EJamSide TargetSide = JamSim::Other(Hazard.Owner);
		const FVector2D TargetPos = Fight.Fighters[TargetSide].Pos;
		if (Hits(Hazard, TargetPos))
		{
			JamFightOps::Hurt(Fight, TargetSide, Hazard.Damage, TargetPos);
		}
	}
}
