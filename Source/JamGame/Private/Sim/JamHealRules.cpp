#include "Sim/JamHealRules.h"

#include "Sim/JamFightOps.h"

namespace
{
bool BossWasHit(const FJamFight& Fight)
{
	return Fight.Events.ContainsByPredicate(
		[](const FJamEvent& Event) { return Event.Type == EJamEvent::Hit && Event.Side == EJamSide::Boss; });
}
} // namespace

void JamHealRules::Drop(FJamFight& Fight, const FJamBalanceData& Balance)
{
	const bool bRoom = Fight.Pickups.Num() < FMath::RoundToInt32(Balance.HealMaxOnFloor);
	if (Fight.Phase != EJamPhase::Fight || Fight.Tick < Fight.NextHealTick || !bRoom || !BossWasHit(Fight))
	{
		return;
	}
	const FJamFighter& Boss = Fight.Fighters[EJamSide::Boss];
	const FJamFighter& Hero = Fight.Fighters[EJamSide::Hero];
	// Towards the hero, so taking it is a step forward and not a walk under the slam; never out of the arena.
	const FVector2D ToHero = (Hero.Pos - Boss.Pos).GetSafeNormal();
	FVector2D Land = Boss.Pos + (ToHero.IsNearlyZero() ? Boss.Facing : ToHero) * Balance.HealToss;
	const double Limit = Balance.ArenaRadius - Balance.HealPickupRadius;
	if (Land.Size() > Limit)
	{
		Land = Land.GetSafeNormal() * Limit;
	}

	FJamPickup Cash;
	Cash.Id = Fight.NextId++;
	if (FVector2D::Distance(Land, Boss.Pos) < Boss.Radius + Balance.HealPickupRadius)
	{
		// T-054: the wall cut the toss short and it would land in him: it goes towards the middle of the arena instead.
		Land = Boss.Pos - Boss.Pos.GetSafeNormal() * Balance.HealToss;
	}
	// T-054: it jumps out of his body, never shows up inside it.
	Cash.From = Boss.Pos + (Land - Boss.Pos).GetSafeNormal() * Boss.Radius;
	Cash.Pos = Land;
	Cash.StartTick = Fight.Tick;
	Cash.LandTick = Fight.Tick + JamSim::Ticks(Balance.HealTossSeconds);
	Cash.DieTick = Cash.LandTick + JamSim::Ticks(Balance.HealLife);
	Fight.Pickups.Add(Cash);
	Fight.NextHealTick = Fight.Tick + JamSim::Ticks(Balance.HealDropInterval);
	JamFightOps::AddEvent(Fight, EJamEvent::HealDropped, EJamSide::Boss, Cash.Id, Land, Balance.HealAmount);
}

void JamHealRules::Collect(FJamFight& Fight, const FJamBalanceData& Balance)
{
	const int32 Tick = Fight.Tick;
	Fight.Pickups.RemoveAll([Tick](const FJamPickup& Cash) { return Tick >= Cash.DieTick; });
	FJamFighter& Hero = Fight.Fighters[EJamSide::Hero];
	for (int32 Index = Fight.Pickups.Num() - 1; Index >= 0; --Index)
	{
		const FJamPickup Cash = Fight.Pickups[Index];
		const bool bReach = FVector2D::Distance(Hero.Pos, Cash.Pos) <= Hero.Radius + Balance.HealPickupRadius;
		// A whole hero walks over it: it waits for when he needs it.
		if (Tick < Cash.LandTick || !bReach || Hero.Health >= Hero.MaxHealth)
		{
			continue;
		}
		const float Before = Hero.Health;
		Hero.Health = FMath::Min(Hero.MaxHealth, Hero.Health + Balance.HealAmount);
		Fight.Pickups.RemoveAt(Index);
		JamFightOps::AddEvent(Fight, EJamEvent::HealTaken, EJamSide::Hero, Cash.Id, Cash.Pos, Hero.Health - Before);
	}
}
