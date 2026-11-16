#include "Sim/JamComboRules.h"

#include "Sim/JamDashRules.h"
#include "Sim/JamParryRules.h"
#include "Sim/JamSlamRules.h"

namespace
{
bool Owns(const FJamFight& Fight, EJamMechanic Mechanic)
{
	return Fight.Ledger.Has(EJamSide::Boss, Mechanic);
}

void Add(TArray<FJamComboLink>& Links, EJamLink Kind, int32 Tick)
{
	Links.Add({ Kind, Tick });
}
} // namespace

TArray<EJamCombo> JamComboRules::Available(const FJamFight& Fight, const FJamBalanceData& Balance)
{
	TArray<EJamCombo> Result;
	if (Fight.Steal.Done < 1)
	{
		return Result;
	}
	const FJamFighter& Boss = Fight.Fighters[EJamSide::Boss];
	const FJamFighter& Hero = Fight.Fighters[EJamSide::Hero];
	const double Distance = FVector2D::Distance(Boss.Pos, Hero.Pos);
	if (Distance <= Balance.SlamRange)
	{
		Result.Add(EJamCombo::Pocket);
	}
	if (Owns(Fight, EJamMechanic::Dash) && JamDashRules::CanStart(Boss.Dash, Fight.Tick) && Distance > Balance.SlamRange)
	{
		Result.Add(EJamCombo::DashSlam);
	}
	if (Owns(Fight, EJamMechanic::Shuriken) && Distance > Balance.KatanaRange + Hero.Radius)
	{
		Result.Add(EJamCombo::FanBurst);
	}
	if (Owns(Fight, EJamMechanic::Parry) && JamParryRules::CanStart(Boss.Parry, Fight.Tick))
	{
		Result.Add(EJamCombo::ShieldFan);
	}
	if (Owns(Fight, EJamMechanic::Ultimate) && Boss.Ultimate.Meter >= 1.f && Distance <= Balance.UltimateRange + Hero.Radius)
	{
		Result.Add(EJamCombo::FanLine);
	}
	if (Owns(Fight, EJamMechanic::Katana))
	{
		Result.Add(EJamCombo::FanRush);
	}
	return Result;
}

EJamCombo JamComboRules::Pick(const TArray<EJamCombo>& Options, EJamCombo Last, FRandomStream& Stream)
{
	TArray<EJamCombo> Fresh = Options;
	if (Fresh.Num() > 1)
	{
		Fresh.Remove(Last);
	}
	return Fresh.IsEmpty() ? EJamCombo::None : Fresh[Stream.RandRange(0, Fresh.Num() - 1)];
}

TArray<FJamComboLink> JamComboRules::Plan(EJamCombo Combo, const FJamFight& Fight, const FJamBalanceData& Balance)
{
	TArray<FJamComboLink> Links;
	const int32 First = Fight.Tick + JamSim::Ticks(Balance.ComboTell);
	const int32 Gap = JamSim::Ticks(Balance.ComboLinkGap);
	switch (Combo)
	{
		case EJamCombo::Pocket:
		{
			// The fan is timed to reach the pocket as the last circle lands: the hero who reads the pattern is already
			// there, and steps out or sends it back.
			const int32 Circles = JamSlamRules::CircleCount(Fight.Steal.Done, Balance);
			const int32 LastHit = First + JamSim::Ticks(Balance.SlamTelegraph + Balance.SlamRipple * (Circles - 1));
			const FJamFighter& Boss = Fight.Fighters[EJamSide::Boss];
			const double Way = FMath::Max(0.0, FVector2D::Distance(Boss.Pos, Fight.Fighters[EJamSide::Hero].Pos) - Boss.Radius);
			const int32 Flight = FMath::RoundToInt32(Way / Balance.MemoSpeed * JamSim::StepsPerSecond);
			Add(Links, EJamLink::Slam, First);
			Add(Links, EJamLink::PocketFan, FMath::Max(First + Gap, LastHit - Flight));
			break;
		}
		case EJamCombo::DashSlam:
			Add(Links, EJamLink::Dash, First);
			Add(Links, EJamLink::Slam, First + JamSim::Ticks(Balance.DashDuration) + Gap);
			break;
		case EJamCombo::FanBurst:
		{
			// His shuriken spacing stretches the burst too: it throws on its own clock, not on his cooldown.
			const int32 ShotGap = JamSim::Ticks(Balance.ComboBurstGap * Balance.BossShurikenCooldownScale);
			Add(Links, EJamLink::Fan, First);
			for (int32 Shot = 0; Shot < FMath::RoundToInt32(Balance.ComboBurstCount); ++Shot)
			{
				Add(Links, EJamLink::Burst, First + Gap + Shot * ShotGap);
			}
			break;
		}
		case EJamCombo::ShieldFan:
			// The fan goes on the step the shield drops: its shrinking ring is the warning.
			Add(Links, EJamLink::Shield, First);
			Add(Links, EJamLink::Fan, First + JamSim::Ticks(Balance.BossParryRaise) + JamSim::Ticks(Balance.BossParryHold));
			break;
		case EJamCombo::FanLine:
			// The line goes along the fan: the step aside that dodges the fan clears the line too.
			Add(Links, EJamLink::Fan, First);
			Add(Links, EJamLink::Line, First + Gap);
			Add(Links, EJamLink::End, First + Gap + JamSim::Ticks(Balance.BossUltimateWindUp) + 1);
			return Links;
		case EJamCombo::FanRush:
			Add(Links, EJamLink::Fan, First);
			Add(Links, EJamLink::Rush, First + Gap);
			Add(Links, EJamLink::End, First + Gap + JamSim::Ticks(Balance.ComboRushSeconds));
			return Links;
		default:
			return Links;
	}
	Add(Links, EJamLink::End, Links.Last().Tick + 1);
	return Links;
}

bool JamComboRules::IsRunning(const FJamFight& Fight)
{
	return Fight.Boss.Combo != EJamCombo::None;
}

const FJamComboLink* JamComboRules::LinkNow(const FJamFight& Fight)
{
	return Fight.Boss.Links.FindByPredicate([&Fight](const FJamComboLink& Link) { return Link.Tick == Fight.Tick; });
}

bool JamComboRules::IsRushing(const FJamFight& Fight)
{
	return Fight.Boss.Links.ContainsByPredicate(
		[&Fight](const FJamComboLink& Link) { return Link.Kind == EJamLink::Rush && Link.Tick <= Fight.Tick; });
}

FVector2D JamComboRules::PocketAim(const FJamFight& Fight, const FJamBalanceData& Balance)
{
	const FVector2D BossPos = Fight.Fighters[EJamSide::Boss].Pos;
	const FVector2D HeroPos = Fight.Fighters[EJamSide::Hero].Pos;
	// The pocket the pattern offered when it went down (T-050 keeps one in reach after the reaction time), seen from where
	// the hero is now: by the time the fan flies, a hero who stood still can reach nothing.
	int32 PatternTick = Fight.Tick;
	for (const FJamHazard& Circle : Fight.Hazards)
	{
		PatternTick = FMath::Min(PatternTick, Circle.StartTick);
	}
	const TOptional<FVector2D> Spot = JamSlamRules::SafeSpot(Fight.Hazards, HeroPos, PatternTick, Balance.SlamReaction, Balance);
	const FVector2D Dir = ((Spot.IsSet() ? *Spot : HeroPos) - BossPos).GetSafeNormal();
	return Dir.IsNearlyZero() ? Fight.Fighters[EJamSide::Boss].Facing : Dir;
}
