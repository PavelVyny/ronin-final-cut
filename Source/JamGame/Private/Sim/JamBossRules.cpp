#include "Sim/JamBossRules.h"

#include "Sim/JamComboRules.h"
#include "Sim/JamDashRules.h"
#include "Sim/JamFightOps.h"
#include "Sim/JamKatanaRules.h"
#include "Sim/JamLoadoutRules.h"
#include "Sim/JamParryRules.h"
#include "Sim/JamShurikenRules.h"
#include "Sim/JamSlamRules.h"

namespace
{
/** The slam grows with the cuts (T-050): the whole pattern goes down at once, he stands until its last circle lands. */
void StartSlam(FJamFight& Fight, const FVector2D& Point, const FJamBalanceData& Balance)
{
	const int32 Count = JamSlamRules::CircleCount(Fight.Steal.Done, Balance);
	const FVector2D BossPos = Fight.Fighters[EJamSide::Boss].Pos;
	int32 LastHit = Fight.Tick;
	for (FJamHazard Circle : JamSlamRules::Pattern(BossPos, Point, Count, Fight.Pickups, Fight.Tick, Fight.Stream, Balance))
	{
		Circle.Id = Fight.NextId++;
		LastHit = FMath::Max(LastHit, Circle.HitTick);
		Fight.Hazards.Add(Circle);
	}
	Fight.Boss.SlamReadyTick = Fight.Tick + JamSim::Ticks(Balance.SlamCooldown);
	Fight.Boss.BusyUntilTick = LastHit + JamSim::Ticks(Balance.BossRecovery);
}

void StartVolley(FJamFight& Fight, const FJamBalanceData& Balance)
{
	FJamShot Memo;
	Memo.Kind = EJamProjectile::Memo;
	Memo.Speed = Balance.MemoSpeed;
	Memo.Radius = Balance.MemoRadius;
	Memo.Damage = Balance.MemoDamage;
	Memo.Life = Balance.MemoLife;
	const FVector2D Aim = Fight.Fighters[EJamSide::Boss].Facing;
	for (const FVector2D& Dir : JamBossRules::VolleyDirections(Aim, FMath::RoundToInt32(Balance.MemoCount), Balance.MemoSpread))
	{
		JamFightOps::Shoot(Fight, EJamSide::Boss, Dir, Memo);
	}
	Fight.Boss.VolleyReadyTick = Fight.Tick + JamSim::Ticks(Balance.VolleyCooldown);
	Fight.Boss.BusyUntilTick = Fight.Tick + JamSim::Ticks(Balance.BossRecovery);
}

/**
 * The stolen dash: out of a katana strike that is about to land, or across the arena at a hero who keeps away.
 * Fills the intent and says true when he dashes on this step.
 */
bool WantsToDash(const FJamFight& Fight, double Distance, const FJamBalanceData& Balance, FJamIntent& Intent)
{
	const FJamFighter& Boss = Fight.Fighters[EJamSide::Boss];
	const FJamFighter& Hero = Fight.Fighters[EJamSide::Hero];
	if (!Fight.Ledger.Has(EJamSide::Boss, EJamMechanic::Dash) || !JamDashRules::CanStart(Boss.Dash, Fight.Tick))
	{
		return false;
	}
	const bool bStrikeComes = Hero.Katana.bHitPending &&
		JamKatanaRules::InArc(Hero.Pos, Hero.Facing, Boss.Pos, Boss.Radius, Balance.KatanaRange, Balance.KatanaHalfArc);
	const bool bHeroIsFar = Distance > Balance.DashDistance + Balance.SlamRange;
	if (!bStrikeComes && !bHeroIsFar)
	{
		return false;
	}
	Intent.Move = bStrikeComes ? -Intent.Aim.Dir : Intent.Aim.Dir;
	Intent.bDash = true;
	return true;
}

/** The stolen Parry (T-038): the shield goes up when a hero's shot would reach him before the stance is over. */
bool WantsTheShield(const FJamFight& Fight, const FJamBalanceData& Balance)
{
	const FJamFighter& Boss = Fight.Fighters[EJamSide::Boss];
	return Fight.Ledger.Has(EJamSide::Boss, EJamMechanic::Parry) && JamParryRules::CanStart(Boss.Parry, Fight.Tick) &&
		JamParryRules::ShotWithin(Fight, EJamSide::Boss, Balance.BossParryRaise + Balance.BossParryHold);
}

/** The intent of a running chain (T-056): he stands and aims, and the link of this step presses its button. */
FJamIntent Steer(const FJamFight& Fight, FJamIntent Intent, const FJamBalanceData& Balance)
{
	const FJamFighter& Boss = Fight.Fighters[EJamSide::Boss];
	const FJamFighter& Hero = Fight.Fighters[EJamSide::Hero];
	const FVector2D HeroVelocity = (Hero.Pos - Hero.PrevPos) / JamSim::StepDt;
	if (JamComboRules::IsRushing(Fight))
	{
		const bool bInReach = FVector2D::Distance(Boss.Pos, Hero.Pos) <= Balance.KatanaRange + Hero.Radius;
		Intent.Move = bInReach ? FVector2D::ZeroVector : Intent.Aim.Dir;
		Intent.bPrimary = bInReach;
	}
	const FJamComboLink* Link = JamComboRules::LinkNow(Fight);
	switch (Link ? Link->Kind : EJamLink::End)
	{
		case EJamLink::Dash:
			Intent.Move = Intent.Aim.Dir;
			Intent.bDash = true;
			break;
		case EJamLink::Shield:
			Intent.bParry = true;
			break;
		case EJamLink::Line:
			Intent.bUltimate = true;
			break;
		case EJamLink::Fan:
			Intent.Aim.Dir =
				JamBossRules::LeadDirection(Boss.Pos, Hero.Pos, HeroVelocity, Balance.MemoSpeed, Boss.Radius + Balance.MemoRadius);
			break;
		case EJamLink::PocketFan:
			Intent.Aim.Dir = JamComboRules::PocketAim(Fight, Balance);
			break;
		case EJamLink::Burst:
			// Led at where the hero runs: the burst punishes the step aside from the fan.
			Intent.Aim.Dir = JamBossRules::LeadDirection(
				Boss.Pos, Hero.Pos, HeroVelocity, Balance.ShurikenSpeed * Balance.BossStolenSpeed, Boss.Radius + Balance.ShurikenRadius);
			break;
		default:
			break;
	}
	return Intent;
}

void StartCombo(FJamFight& Fight, const FJamBalanceData& Balance)
{
	const EJamCombo Combo = JamComboRules::Pick(JamComboRules::Available(Fight, Balance), Fight.Boss.LastCombo, Fight.Stream);
	if (Combo != EJamCombo::None)
	{
		Fight.Boss.Combo = Combo;
		Fight.Boss.Links = JamComboRules::Plan(Combo, Fight, Balance);
	}
}

/** Fires his own attacks of the link of this step; the stolen ones went through the buttons. The end is his pause. */
void RunLink(FJamFight& Fight, const FJamBalanceData& Balance)
{
	const FJamComboLink* Link = JamComboRules::LinkNow(Fight);
	switch (Link ? Link->Kind : EJamLink::Rush)
	{
		case EJamLink::Slam:
			StartSlam(Fight, Fight.Fighters[EJamSide::Hero].Pos, Balance);
			break;
		case EJamLink::Fan:
		case EJamLink::PocketFan:
			StartVolley(Fight, Balance);
			break;
		case EJamLink::Burst:
		{
			// The burst is quicker than his cooldown: the chain throws on its own clock.
			Fight.Fighters[EJamSide::Boss].Shuriken.ReadyTick = Fight.Tick;
			JamShurikenRules::Throw(Fight, EJamSide::Boss, JamLoadoutRules::InHandsOf(EJamSide::Boss, Balance));
			break;
		}
		case EJamLink::End:
			Fight.Boss.LastCombo = Fight.Boss.Combo;
			Fight.Boss.Combo = EJamCombo::None;
			Fight.Boss.Links.Reset();
			Fight.Boss.ComboReadyTick = Fight.Tick + JamSim::Ticks(Balance.ComboCooldown);
			Fight.Boss.BusyUntilTick = FMath::Max(Fight.Boss.BusyUntilTick, Fight.Tick + JamSim::Ticks(Balance.BossRecovery));
			break;
		default:
			break;
	}
}

/** The stolen Ultimate (T-038): his clock filled the meter and the hero is within the line's reach. */
bool WantsToSlash(const FJamFight& Fight, double Distance, const FJamBalanceData& Balance)
{
	const FJamFighter& Boss = Fight.Fighters[EJamSide::Boss];
	return Fight.Ledger.Has(EJamSide::Boss, EJamMechanic::Ultimate) && Boss.Ultimate.Meter >= 1.f &&
		Distance <= Balance.UltimateRange + Fight.Fighters[EJamSide::Hero].Radius;
}

void PickAnAttack(const FJamFight& Fight, double Distance, const FJamBalanceData& Balance, FJamIntent& Intent);
} // namespace

FJamIntent JamBossRules::Decide(const FJamFight& Fight, const FJamBalanceData& Balance)
{
	const FJamFighter& Boss = Fight.Fighters[EJamSide::Boss];
	const FJamFighter& Hero = Fight.Fighters[EJamSide::Hero];
	const FVector2D To = Hero.Pos - Boss.Pos;
	const double Distance = To.Size();

	FJamIntent Intent;
	Intent.Aim.Dir = Distance > UE_KINDA_SMALL_NUMBER ? To / Distance : Boss.Facing;
	Intent.Aim.Point = Hero.Pos;
	if (Fight.Phase != EJamPhase::Fight)
	{
		return Intent;
	}
	if (JamComboRules::IsRunning(Fight))
	{
		return Steer(Fight, Intent, Balance);
	}
	if (IsBusy(Fight))
	{
		return Intent;
	}
	// What he stole to defend and to finish comes before his own attacks: both stop him, so they are the openings too.
	if (WantsTheShield(Fight, Balance))
	{
		Intent.bParry = true;
		return Intent;
	}
	// T-056: from the first cut a chain is his first choice whenever one is ready.
	if (Fight.Tick >= Fight.Boss.ComboReadyTick && !JamComboRules::Available(Fight, Balance).IsEmpty())
	{
		Intent.bCombo = true;
		return Intent;
	}
	if (WantsToSlash(Fight, Distance, Balance))
	{
		Intent.bUltimate = true;
		return Intent;
	}
	if (!WantsToDash(Fight, Distance, Balance, Intent))
	{
		PickAnAttack(Fight, Distance, Balance, Intent);
	}
	return Intent;
}

namespace
{
/** One attack of his own at a time, or the walk at the hero; the stolen shuriken fills the gaps. */
void PickAnAttack(const FJamFight& Fight, double Distance, const FJamBalanceData& Balance, FJamIntent& Intent)
{
	const FJamFighter& Boss = Fight.Fighters[EJamSide::Boss];
	const FJamFighter& Hero = Fight.Fighters[EJamSide::Hero];
	const bool bInBladeReach = Distance <= Balance.KatanaRange + Hero.Radius;
	// One attack of his own at a time, the slam first: a hero who came close must pay attention.
	if (Distance <= Balance.SlamRange && Fight.Tick >= Fight.Boss.SlamReadyTick)
	{
		Intent.bSlam = true;
	}
	else if (bInBladeReach && Fight.Ledger.Has(EJamSide::Boss, EJamMechanic::Katana))
	{
		Intent.bPrimary = true;
	}
	else if (Fight.Tick >= Fight.Boss.VolleyReadyTick)
	{
		// The volley goes where the hero will be: running in one direction is not a way to dodge it.
		const FVector2D HeroVelocity = (Hero.Pos - Hero.PrevPos) / JamSim::StepDt;
		Intent.Aim.Dir = JamBossRules::LeadDirection(Boss.Pos, Hero.Pos, HeroVelocity, Balance.MemoSpeed, Boss.Radius + Balance.MemoRadius);
		Intent.bVolley = true;
	}
	else if (Distance > Boss.Radius + Hero.Radius + 1.0)
	{
		Intent.Move = Intent.Aim.Dir;
	}
	// The stolen shuriken fills the gaps between his own attacks: it is thrown at a hero who keeps away. Never on
	// the step of a volley: that aim is led, and the shuriken stays a straight throw.
	Intent.bSecondary = !bInBladeReach && !Intent.bSlam && !Intent.bVolley && Fight.Ledger.Has(EJamSide::Boss, EJamMechanic::Shuriken);
}
} // namespace

bool JamBossRules::IsBusy(const FJamFight& Fight)
{
	// He also stands through the wind-up of the stolen katana (the slow blade is the warning), behind his raised shield
	// and through the wind-up of his Ultimate (T-038).
	// A running chain (T-056) is never "busy": it steers him step by step itself.
	const FJamFighter& Boss = Fight.Fighters[EJamSide::Boss];
	const bool bShield = Fight.Tick < Boss.Parry.ActiveUntilTick;
	if (Fight.Phase == EJamPhase::Stunned)
	{
		return true;
	}
	return !JamComboRules::IsRunning(Fight) &&
		(Fight.Tick < Fight.Boss.BusyUntilTick || Boss.Katana.bHitPending || bShield || Boss.Ultimate.bPending);
}

FVector2D JamBossRules::LeadDirection(
	const FVector2D& From, const FVector2D& TargetPos, const FVector2D& TargetVelocity, float Speed, float HeadStart)
{
	const FVector2D To = TargetPos - From;
	const FVector2D Direct = To.GetSafeNormal();
	// The two meet when |To + TargetVelocity * T| = HeadStart + Speed * T: a quadratic in T.
	const double A = TargetVelocity.SizeSquared() - Speed * Speed;
	const double B = 2.0 * (FVector2D::DotProduct(To, TargetVelocity) - Speed * HeadStart);
	const double C = To.SizeSquared() - HeadStart * HeadStart;
	if (A > -UE_KINDA_SMALL_NUMBER || C <= 0.0)
	{
		// A target as fast as the projectile, or one already at the muzzle: no lead is worth guessing.
		return Direct;
	}
	// A is negative and C positive: the roots have opposite signs, this one is the future.
	const double Seconds = (-B - FMath::Sqrt(B * B - 4.0 * A * C)) / (2.0 * A);
	return (To + TargetVelocity * Seconds).GetSafeNormal();
}

TArray<FVector2D> JamBossRules::VolleyDirections(const FVector2D& AimDir, int32 Count, float SpreadDeg)
{
	TArray<FVector2D> Directions;
	const FVector2D Dir = AimDir.GetSafeNormal();
	for (int32 Index = 0; Index < Count; ++Index)
	{
		const float Angle = Count > 1 ? SpreadDeg * (static_cast<float>(Index) / (Count - 1) - 0.5f) : 0.f;
		Directions.Add(Dir.GetRotated(Angle));
	}
	return Directions;
}

void JamBossRules::Act(FJamFight& Fight, const FJamIntent& Intent, const FJamBalanceData& Balance)
{
	// The finisher of the stolen katana ends with the same pause as his own attacks: the player's opening.
	const FJamKatanaState& Katana = Fight.Fighters[EJamSide::Boss].Katana;
	if (Katana.bHitPending && JamKatanaRules::IsFinisher(Katana.Strike))
	{
		const int32 RestUntil = Katana.HitTick + JamSim::Ticks(Balance.BossRecovery);
		Fight.Boss.BusyUntilTick = FMath::Max(Fight.Boss.BusyUntilTick, RestUntil);
	}
	if (Fight.Phase == EJamPhase::Fight && JamComboRules::IsRunning(Fight))
	{
		RunLink(Fight, Balance);
		return;
	}
	if (IsBusy(Fight))
	{
		return;
	}
	if (Intent.bCombo)
	{
		StartCombo(Fight, Balance);
	}
	else if (Intent.bSlam && Fight.Tick >= Fight.Boss.SlamReadyTick)
	{
		StartSlam(Fight, Intent.Aim.Point, Balance);
	}
	else if (Intent.bVolley && Fight.Tick >= Fight.Boss.VolleyReadyTick)
	{
		StartVolley(Fight, Balance);
	}
}
