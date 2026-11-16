#include "Sim/JamSlamRules.h"

namespace
{
// How far outside a circle a spot must be to count as safe: the walk is checked between fixed steps.
constexpr double SafeMargin = 20.0;
// The search for a safe spot: rays every this many degrees, points every this many cm along each.
constexpr float SearchDegrees = 10.f;
constexpr double SearchStep = 20.0;

FJamHazard Circle(const FVector2D& Pos, float Radius, int32 Order, int32 Tick, const FJamBalanceData& Balance)
{
	FJamHazard Result;
	Result.Owner = EJamSide::Boss;
	Result.Pos = Pos;
	Result.Radius = Radius;
	Result.Damage = Balance.SlamDamage;
	Result.StartTick = Tick;
	Result.HitTick = Tick + JamSim::Ticks(Balance.SlamTelegraph) + Order * JamSim::Ticks(Balance.SlamRipple);
	return Result;
}

/** Moves a circle off every cash stack, with room to stand on it. */
void KeepOffTheCash(FJamHazard& Hazard, const TArray<FJamPickup>& Pickups, const FVector2D& Fallback, const FJamBalanceData& Balance)
{
	for (const FJamPickup& Cash : Pickups)
	{
		const double Clear = Hazard.Radius + Balance.HealPickupRadius;
		const FVector2D From = Hazard.Pos - Cash.Pos;
		if (From.SizeSquared() < Clear * Clear)
		{
			const FVector2D Dir = From.IsNearlyZero() ? Fallback : From.GetSafeNormal();
			Hazard.Pos = Cash.Pos + Dir * Clear;
		}
	}
}

/** True when a circle at Pos is on the floor, off the cash and keeps the gap to every one of the Placed circles. */
bool HasRoom(const FVector2D& Pos, double Radius, const TArray<FJamHazard>& Placed, const TArray<FJamPickup>& Pickups,
	const FJamBalanceData& Balance)
{
	if (Pos.Size() > Balance.ArenaRadius)
	{
		return false;
	}
	for (const FJamHazard& Other : Placed)
	{
		if (FVector2D::Distance(Pos, Other.Pos) - Radius - Other.Radius < Balance.SlamGap)
		{
			return false;
		}
	}
	for (const FJamPickup& Cash : Pickups)
	{
		if (FVector2D::Distance(Pos, Cash.Pos) < Radius + Balance.HealPickupRadius)
		{
			return false;
		}
	}
	return true;
}

/**
 * The place nearest to Dir on a ring of Distance around Anchor where a circle has room: the wanted direction first,
 * then turning further either way, then a wider ring. Unset when the floor has no such place.
 */
TOptional<FVector2D> FindRoom(const FVector2D& Anchor, const FVector2D& Dir, double Distance, double Radius,
	const TArray<FJamHazard>& Placed, const TArray<FJamPickup>& Pickups, const FJamBalanceData& Balance)
{
	constexpr float TurnStep = 15.f;
	constexpr int32 Rings = 8;
	constexpr double RingStep = 100.0;
	// Each ring is tried at 0, +15, -15, +30, -30 ... degrees off the wanted direction.
	constexpr int32 TriesPerRing = 2 * static_cast<int32>(180.f / TurnStep) + 1;
	for (int32 Try = 0; Try < Rings * TriesPerRing; ++Try)
	{
		const int32 Ring = Try / TriesPerRing;
		const int32 Step = Try % TriesPerRing;
		const float Turn = ((Step + 1) / 2) * TurnStep * (Step % 2 == 0 ? -1.f : 1.f);
		const FVector2D Pos = Anchor + Dir.GetRotated(Turn) * (Distance + Ring * RingStep);
		if (HasRoom(Pos, Radius, Placed, Pickups, Balance))
		{
			return Pos;
		}
	}
	return {};
}

/**
 * Five circles (T-052): two at the Publisher's flanks, then the one on the hero, then two wide behind the hero on
 * either side of his way back. Each keeps the gap to the ones placed before it or is left out; the one on the hero is
 * placed first and always stays. Returned in landing order, from him outwards.
 */
TArray<FJamHazard> SpreadPattern(const FVector2D& BossPos, const FVector2D& HeroPos, const FVector2D& Away, const float (&Jitters)[3],
	const TArray<FJamPickup>& Pickups, int32 Tick, const FJamBalanceData& Balance, int32& OnHero)
{
	const double NearDistance = Balance.SlamSideRadius + 0.5 * Balance.SlamGap;
	const double WideDistance = Balance.SlamRadius + Balance.SlamSideRadius + Balance.SlamGap;
	const FVector2D Back = -Away;
	struct FWanted
	{
		int32 Rank;
		FVector2D Anchor;
		FVector2D Dir;
		double Distance;
	};
	// Landing rank and wanted place of each circle around its anchor, in the order they claim the floor.
	const FWanted Wanted[] = {
		{ 0, BossPos, Away.GetRotated(90.f + Jitters[0]), NearDistance },
		{ 1, BossPos, Away.GetRotated(-90.f + Jitters[0]), NearDistance },
		{ 3, HeroPos, Back.GetRotated(Balance.SlamSpreadAngle + Jitters[1]), WideDistance },
		{ 4, HeroPos, Back.GetRotated(-Balance.SlamSpreadAngle + Jitters[2]), WideDistance },
	};
	FJamHazard OnTheHero = Circle(HeroPos, Balance.SlamRadius, 0, Tick, Balance);
	KeepOffTheCash(OnTheHero, Pickups, Away, Balance);
	TArray<FJamHazard> Placed = { OnTheHero };
	TArray<int32> Ranks = { 2 };
	for (const FWanted& Want : Wanted)
	{
		const TOptional<FVector2D> Pos = FindRoom(Want.Anchor, Want.Dir, Want.Distance, Balance.SlamSideRadius, Placed, Pickups, Balance);
		if (Pos.IsSet())
		{
			Placed.Add(Circle(*Pos, Balance.SlamSideRadius, 0, Tick, Balance));
			Ranks.Add(Want.Rank);
		}
	}
	TArray<FJamHazard> Result;
	for (int32 Rank = 0; Rank <= UE_ARRAY_COUNT(Wanted); ++Rank)
	{
		const int32 Index = Ranks.Find(Rank);
		if (Index != INDEX_NONE)
		{
			OnHero = Rank == 2 ? Result.Num() : OnHero;
			Result.Add(Circle(Placed[Index].Pos, Placed[Index].Radius, Result.Num(), Tick, Balance));
		}
	}
	return Result;
}

/** Where a walker who sets off after Reaction seconds towards Spot is Seconds after the start. */
FVector2D WalkerAt(const FVector2D& Pos, const FVector2D& Spot, double Seconds, double Reaction, double Speed)
{
	const FVector2D To = Spot - Pos;
	const double Walked = FMath::Max(0.0, Seconds - Reaction) * Speed;
	return Walked >= To.Size() ? Spot : Pos + To.GetSafeNormal() * Walked;
}

bool IsSafe(const TArray<FJamHazard>& Hazards, const FVector2D& Pos, const FVector2D& Spot, int32 Tick, double Reaction,
	const FJamBalanceData& Balance)
{
	for (const FJamHazard& Hazard : Hazards)
	{
		const double Seconds = (Hazard.HitTick - Tick) * JamSim::StepDt;
		const FVector2D There = WalkerAt(Pos, Spot, Seconds, Reaction, Balance.HeroSpeed);
		if (FVector2D::Distance(There, Hazard.Pos) <= Hazard.Radius + SafeMargin)
		{
			return false;
		}
	}
	return true;
}
} // namespace

int32 JamSlamRules::CircleCount(int32 CutsDone, const FJamBalanceData& Balance)
{
	const float Count = CutsDone <= 0 ? Balance.SlamCirclesUncut
		: CutsDone == 1               ? Balance.SlamCirclesFirstCut
									  : Balance.SlamCirclesSecondCut;
	return FMath::Max(1, FMath::RoundToInt32(Count));
}

TArray<FJamHazard> JamSlamRules::Pattern(const FVector2D& BossPos, const FVector2D& HeroPos, int32 Count, const TArray<FJamPickup>& Pickups,
	int32 Tick, FRandomStream& Stream, const FJamBalanceData& Balance)
{
	TArray<FJamHazard> Result;
	if (Count <= 1)
	{
		// The old slam: one circle on the hero and no draw from the stream.
		Result.Add(Circle(HeroPos, Balance.SlamRadius, 0, Tick, Balance));
		KeepOffTheCash(Result[0], Pickups, (HeroPos - BossPos).GetSafeNormal(), Balance);
		return Result;
	}
	const FVector2D Away = (HeroPos - BossPos).IsNearlyZero() ? FVector2D(1.0, 0.0) : (HeroPos - BossPos).GetSafeNormal();
	// The way back first, then one side and the other; every cutter is drawn so the stream moves the same for any count.
	const float Jitter = Balance.SlamJitter;
	const float Jitters[] = { Stream.FRandRange(-Jitter, Jitter), Stream.FRandRange(-Jitter, Jitter), Stream.FRandRange(-Jitter, Jitter) };
	if (Count >= 5)
	{
		int32 OnHero = 0;
		Result = SpreadPattern(BossPos, HeroPos, Away, Jitters, Pickups, Tick, Balance, OnHero);
		// The wide ones, then the ones at his flanks go while they shut every way out; the one on the hero stays.
		while (Result.Num() > 1 && !SafeSpot(Result, HeroPos, Tick, Balance.SlamReaction, Balance).IsSet())
		{
			const int32 Last = Result.Num() - 1 == OnHero ? Result.Num() - 2 : Result.Num() - 1;
			Result.RemoveAt(Last);
			OnHero -= Last < OnHero ? 1 : 0;
		}
		return Result;
	}
	const float Angles[] = { Jitters[0], Balance.SlamFlankAngle + Jitters[1], -Balance.SlamFlankAngle + Jitters[2] };
	Result.Add(Circle(BossPos, Balance.SlamSideRadius, 0, Tick, Balance));
	Result.Add(Circle(HeroPos, Balance.SlamRadius, 1, Tick, Balance));
	const double Room = Balance.ArenaRadius;
	for (int32 Index = 0; Index < UE_ARRAY_COUNT(Angles) && Result.Num() < Count; ++Index)
	{
		FVector2D Pos = HeroPos + Away.GetRotated(Angles[Index]) * Balance.SlamCutterDistance;
		// A cutter past the wall would land unseen: it is pulled back onto the floor.
		Pos = Pos.Size() > Room ? Pos.GetSafeNormal() * Room : Pos;
		Result.Add(Circle(Pos, Balance.SlamSideRadius, Result.Num(), Tick, Balance));
	}
	for (FJamHazard& Hazard : Result)
	{
		KeepOffTheCash(Hazard, Pickups, Away, Balance);
	}
	// The last cutters go first while they shut every way out; the circles on him and on the hero always stay.
	while (Result.Num() > 2 && !SafeSpot(Result, HeroPos, Tick, Balance.SlamReaction, Balance).IsSet())
	{
		Result.Pop();
	}
	return Result;
}

TOptional<FVector2D> JamSlamRules::SafeSpot(
	const TArray<FJamHazard>& Hazards, const FVector2D& Pos, int32 Tick, float ReactionSeconds, const FJamBalanceData& Balance)
{
	int32 LastHit = Tick;
	for (const FJamHazard& Hazard : Hazards)
	{
		LastHit = FMath::Max(LastHit, Hazard.HitTick);
	}
	const double Reach = FMath::Max(0.0, (LastHit - Tick) * JamSim::StepDt - ReactionSeconds) * Balance.HeroSpeed;
	const double Room = Balance.ArenaRadius - Balance.HeroRadius;
	// Nearest first: ring by ring outwards, every ray on a ring.
	for (double Distance = 0.0; Distance <= Reach; Distance += SearchStep)
	{
		for (float Angle = 0.f; Angle < 360.f; Angle += SearchDegrees)
		{
			const FVector2D Spot = Pos + FVector2D(1.0, 0.0).GetRotated(Angle) * Distance;
			if (Spot.Size() <= Room && IsSafe(Hazards, Pos, Spot, Tick, ReactionSeconds, Balance))
			{
				return Spot;
			}
			if (Distance == 0.0)
			{
				break;
			}
		}
	}
	return {};
}
