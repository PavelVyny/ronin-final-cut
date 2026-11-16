#include "Sim/JamBotRules.h"

#include "Sim/JamBossRules.h"
#include "Sim/JamFightRules.h"
#include "Sim/JamParryRules.h"
#include "Sim/JamSlamRules.h"
#include "Sim/JamUltimateRules.h"

namespace
{
// Share of the range over which the bot goes from "hold the distance" to a full run in or out.
constexpr float RangeSoftness = 0.25f;
// The bot holds its distance at this share of the slam range: too far to be slammed, close enough to run in.
constexpr float RangeOverSlam = 1.5f;
// How far outside a warning circle the bot wants to be before it stops running.
constexpr float CircleMargin = 30.f;
// Closer than this to its safe spot the bot stands.
constexpr double SpotReached = 5.0;
// A projectile that arrives sooner than this is dodged with the dash.
constexpr float DodgeSeconds = 0.15f;
constexpr float DodgeMargin = 10.f;
// How far outside the reach of a blade on its way the bot wants to be.
constexpr float BladeMargin = 60.f;
// How far off the line of an Ultimate winding up the bot wants to be.
constexpr float LaneMargin = 40.f;
// How far ahead the bot looks along its circle for a projectile to turn from, and how wide a berth it wants.
constexpr float TurnSeconds = 1.5f;
constexpr float TurnMargin = 30.f;
// The room the bot keeps from the arena wall, in blast radii of a shuriken: what flies past it goes off there.
constexpr float WallRoomInBlasts = 2.f;
// How hard the wall turns the bot back at its very foot, against a move of length one.
constexpr float WallPush = 2.f;

/** True while the enemy's katana strike is on its way and the fighter stands where it lands. */
bool IsUnderTheBlade(const FJamFight& Fight, EJamSide Side, const FJamBalanceData& Balance)
{
	const FJamFighter& Me = Fight.Fighters[Side];
	const FJamFighter& Enemy = Fight.Fighters[JamSim::Other(Side)];
	const double Reach = Balance.KatanaRange + Balance.KatanaStep + Me.Radius + BladeMargin;
	return Enemy.Katana.bHitPending && FVector2D::Distance(Me.Pos, Enemy.Pos) < Reach;
}

/** Writes down what the Publisher took on the last step. */
void NoteTheCuts(const FJamFight& Fight, FJamBotRun& Run)
{
	for (const FJamEvent& Event : Fight.Events)
	{
		if (Event.Type == EJamEvent::StealResolved && Event.Side == EJamSide::Boss)
		{
			Run.Stolen.Add(static_cast<EJamMechanic>(FMath::RoundToInt32(Event.Amount)));
		}
	}
}

/** The enemy's warning circle the fighter stands in, if there is one. */
const FJamHazard* CircleOver(const FJamFight& Fight, EJamSide Side)
{
	const FVector2D Pos = Fight.Fighters[Side].Pos;
	for (const FJamHazard& Hazard : Fight.Hazards)
	{
		if (Hazard.Owner != Side && FVector2D::Distance(Hazard.Pos, Pos) < Hazard.Radius + CircleMargin)
		{
			return &Hazard;
		}
	}
	return nullptr;
}

/** The enemy's warning circles on the floor. */
TArray<FJamHazard> EnemyCircles(const FJamFight& Fight, EJamSide Side)
{
	TArray<FJamHazard> Result;
	for (const FJamHazard& Hazard : Fight.Hazards)
	{
		if (Hazard.Owner != Side)
		{
			Result.Add(Hazard);
		}
	}
	return Result;
}

/** Steers the fighter clear of the enemy's warning circles; says true when there are circles to steer by. */
bool DodgeCircles(const FJamFight& Fight, EJamSide Side, const FJamBalanceData& Balance, FJamIntent& Intent)
{
	const FJamFighter& Me = Fight.Fighters[Side];
	const TArray<FJamHazard> Circles = EnemyCircles(Fight, Side);
	const FJamHazard* Circle = CircleOver(Fight, Side);
	if (Circles.IsEmpty())
	{
		return false;
	}
	if (const TOptional<FVector2D> Spot = JamSlamRules::SafeSpot(Circles, Me.Pos, Fight.Tick, 0.f, Balance))
	{
		// While his circles are down the bot goes to the nearest spot none of them catches it on the way to (T-050), and
		// waits there; the old rule left only the circle it stood in, often into the next one.
		const FVector2D To = *Spot - Me.Pos;
		Intent.Move = To.Size() > SpotReached ? To.GetSafeNormal() : FVector2D::ZeroVector;
		Intent.bDash = Intent.bDash || Circle != nullptr;
		return true;
	}
	if (!Circle)
	{
		return false;
	}
	// Too late to walk anywhere: out of the circle by the shortest way and the dash; from its very centre any way is as
	// good, so away from the enemy.
	const FVector2D Away = Me.Pos - Circle->Pos;
	Intent.Move = Away.IsNearlyZero() ? -Intent.Aim.Dir : Away.GetSafeNormal();
	Intent.bDash = true;
	return true;
}

/** Seconds until a projectile ends on its own: at the arena wall or when its life runs out. */
double SecondsToTheEnd(const FJamProjectile& Flying, int32 Tick, float ArenaRadius)
{
	const double Life = FMath::Max(Flying.DieTick - Tick, 0) * JamSim::StepDt;
	const double SpeedSquared = Flying.Vel.SizeSquared();
	const double Inside = FMath::Square(static_cast<double>(ArenaRadius)) - Flying.Pos.SizeSquared();
	if (SpeedSquared < UE_KINDA_SMALL_NUMBER || Inside <= 0.0)
	{
		return Inside <= 0.0 ? 0.0 : Life;
	}
	// |Pos + Vel * T| = ArenaRadius: from the inside one root is ahead.
	const double Along = FVector2D::DotProduct(Flying.Pos, Flying.Vel);
	const double ToTheWall = (-Along + FMath::Sqrt(Along * Along + SpeedSquared * Inside)) / SpeedSquared;
	return FMath::Min(Life, ToTheWall);
}

/** True when an explosive projectile goes off within the turn time next to where the fighter will be by then. */
bool BlastReaches(
	const FJamFight& Fight, const FJamProjectile& Flying, EJamSide Side, const FVector2D& Velocity, const FJamBalanceData& Balance)
{
	const FJamFighter& Me = Fight.Fighters[Side];
	const double Seconds = SecondsToTheEnd(Flying, Fight.Tick, Balance.ArenaRadius);
	const FVector2D Blast = Flying.Pos + Flying.Vel * Seconds;
	const FVector2D Mine = Me.Pos + Velocity * Seconds;
	return Seconds <= TurnSeconds && FVector2D::Distance(Blast, Mine) < Balance.ShurikenExplosionRadius + Me.Radius + TurnMargin;
}

/**
 * True when the fighter, moving on at Velocity, runs into an enemy projectile, or into the blast of an explosive one,
 * within the turn time.
 */
bool CourseIsHit(const FJamFight& Fight, EJamSide Side, const FVector2D& Velocity, const FJamBalanceData& Balance)
{
	const FJamFighter& Me = Fight.Fighters[Side];
	for (const FJamProjectile& Flying : Fight.Projectiles)
	{
		if (Flying.Owner == Side)
		{
			continue;
		}
		if (Flying.bExplosive && BlastReaches(Fight, Flying, Side, Velocity, Balance))
		{
			return true;
		}
		// The moment the two are closest, seen from the fighter: the projectile closes in at the difference of the speeds.
		const FVector2D Gap = Flying.Pos - Me.Pos;
		const FVector2D Closing = Flying.Vel - Velocity;
		const double ClosingSquared = Closing.SizeSquared();
		const double Seconds = ClosingSquared > UE_KINDA_SMALL_NUMBER
			? FMath::Clamp(-FVector2D::DotProduct(Gap, Closing) / ClosingSquared, 0.0, static_cast<double>(TurnSeconds))
			: 0.0;
		if ((Gap + Closing * Seconds).Size() < Me.Radius + Flying.Radius + TurnMargin)
		{
			return true;
		}
	}
	return false;
}

/**
 * The way around the enemy: the circle keeps the direction the fighter already runs in, and turns back when that way
 * runs into a projectile and the other one does not.
 */
FVector2D CircleMove(const FJamFight& Fight, EJamSide Side, const FJamIntent& Kiting, const FJamBalanceData& Balance)
{
	const FJamFighter& Me = Fight.Fighters[Side];
	const FVector2D Sideways(-Kiting.Aim.Dir.Y, Kiting.Aim.Dir.X);
	const FVector2D Left = Kiting.Move;
	const FVector2D Right = Left - 2.0 * FVector2D::DotProduct(Left, Sideways) * Sideways;
	const bool bGoesRight = FVector2D::DotProduct(Me.Pos - Me.PrevPos, Sideways) < -UE_KINDA_SMALL_NUMBER;
	const FVector2D Course = bGoesRight ? Right : Left;
	const FVector2D Turned = bGoesRight ? Left : Right;
	const bool bTurn = CourseIsHit(Fight, Side, Course * Me.Speed, Balance) && !CourseIsHit(Fight, Side, Turned * Me.Speed, Balance);
	return bTurn ? Turned : Course;
}

/** The move with the arena wall in mind: the closer the wall, the more the fighter turns back into the open. */
FVector2D OffTheWall(const FJamFighter& Me, const FVector2D& Move, const FJamBalanceData& Balance)
{
	const double Room = WallRoomInBlasts * Balance.ShurikenExplosionRadius;
	const double FromCentre = Me.Pos.Size();
	// Nothing where the room begins, one where the fighter touches the wall.
	const double Depth = FMath::Min((FromCentre + Me.Radius + Room - Balance.ArenaRadius) / Room, 1.0);
	if (FromCentre < UE_KINDA_SMALL_NUMBER || Depth <= 0.0)
	{
		return Move;
	}
	const FVector2D Outward = Me.Pos / FromCentre;
	const FVector2D Pushed = (Move - Outward * (WallPush * Depth)).GetSafeNormal();
	return Pushed.IsNearlyZero() ? -Outward : Pushed;
}

/**
 * T-038: on the line of the enemy's Ultimate winding up, the fighter walks off it sideways, the short way, and dashes
 * when it is about to land. Says true when it does.
 */
bool StepOffTheLine(const FJamFight& Fight, EJamSide Side, const FJamBalanceData& Balance, FJamIntent& Intent)
{
	const FJamFighter& Me = Fight.Fighters[Side];
	const FJamFighter& Enemy = Fight.Fighters[JamSim::Other(Side)];
	const FJamUltimateState& Line = Enemy.Ultimate;
	if (!Line.bPending ||
		!JamUltimateRules::OnLine(Enemy.Pos, Line.Dir, Me.Pos, Me.Radius + LaneMargin, Balance.UltimateRange, Balance.UltimateHalfWidth))
	{
		return false;
	}
	const FVector2D Sideways(-Line.Dir.Y, Line.Dir.X);
	Intent.Move = FVector2D::DotProduct(Me.Pos - Enemy.Pos, Sideways) >= 0.0 ? Sideways : -Sideways;
	Intent.bDash = Line.SlashTick - Fight.Tick <= JamSim::Ticks(DodgeSeconds);
	return true;
}

/** Enemy projectiles that will reach the fighter within the dodge time. */
int32 Threats(const FJamFight& Fight, EJamSide Side)
{
	const FJamFighter& Me = Fight.Fighters[Side];
	int32 Count = 0;
	for (const FJamProjectile& Flying : Fight.Projectiles)
	{
		if (Flying.Owner == Side)
		{
			continue;
		}
		const FVector2D Closest = FMath::ClosestPointOnSegment2D(Me.Pos, Flying.Pos, Flying.Pos + Flying.Vel * DodgeSeconds);
		Count += FVector2D::Distance(Closest, Me.Pos) < Me.Radius + Flying.Radius + DodgeMargin ? 1 : 0;
	}
	return Count;
}

/** The landed heal the hero goes for, if he has lost at least what it gives back; none for the Publisher. */
const FJamPickup* CashToTake(const FJamFight& Fight, EJamSide Side, const FJamBalanceData& Balance)
{
	const FJamFighter& Me = Fight.Fighters[Side];
	if (Side != EJamSide::Hero || Me.Health > Me.MaxHealth - Balance.HealAmount)
	{
		return nullptr;
	}
	return Fight.Pickups.FindByPredicate([&Fight](const FJamPickup& Cash) { return Fight.Tick >= Cash.LandTick; });
}
} // namespace

FJamIntent JamBotRules::Kite(const FJamFight& Fight, EJamSide Side, float Range)
{
	const FJamFighter& Me = Fight.Fighters[Side];
	const FJamFighter& Target = Fight.Fighters[JamSim::Other(Side)];
	const FVector2D To = Target.Pos - Me.Pos;
	const double Distance = To.Size();
	const FVector2D Dir = Distance > UE_KINDA_SMALL_NUMBER ? To / Distance : Me.Facing;
	const FVector2D Sideways(-Dir.Y, Dir.X);
	const double Radial = FMath::Clamp((Distance - Range) / (Range * RangeSoftness), -1.0, 1.0);

	FJamIntent Intent;
	Intent.Move = (Dir * Radial + Sideways).GetSafeNormal();
	Intent.Aim.Dir = Dir;
	Intent.Aim.Point = Target.Pos;
	Intent.bPrimary = true;
	return Intent;
}

FJamIntent JamBotRules::Play(const FJamFight& Fight, EJamSide Side, const FJamBalanceData& Balance)
{
	const FJamFighter& Me = Fight.Fighters[Side];
	const FJamFighter& Target = Fight.Fighters[JamSim::Other(Side)];
	const bool bHasKatana = Fight.Ledger.Has(Side, EJamMechanic::Katana);
	const bool bInReach = FVector2D::Distance(Me.Pos, Target.Pos) <= Balance.KatanaRange + Target.Radius;

	FJamIntent Intent = Kite(Fight, Side, Balance.SlamRange * RangeOverSlam);
	Intent.Move = OffTheWall(Me, CircleMove(Fight, Side, Intent, Balance), Balance);
	if (const FJamPickup* Cash = CashToTake(Fight, Side, Balance))
	{
		// A heal on the floor is worth a detour once it would heal in full; a dodge below still comes first.
		Intent.Move = (Cash->Pos - Me.Pos).GetSafeNormal();
	}
	// The katana is not swung at the air; the ball on the same button is shot from anywhere, but never into the enemy's
	// raised shield (T-038): it would only come back.
	const bool bShieldUp = JamParryRules::IsRaising(Target.Parry, Fight.Tick) || JamParryRules::IsActive(Target.Parry, Fight.Tick);
	Intent.bPrimary = bHasKatana ? bInReach : !bShieldUp;
	Intent.bSecondary = !bShieldUp;
	// A lone projectile about to land is parried when the parry is at hand: it goes back and the dash stays for the slam.
	// Several at once are dashed through: one window does not cover the whole fan.
	const int32 ThreatCount = Threats(Fight, Side);
	const bool bCanParry = Fight.Ledger.Has(Side, EJamMechanic::Parry) && JamParryRules::CanStart(Me.Parry, Fight.Tick);
	const bool bParryNow = ThreatCount == 1 && bCanParry;
	// The hero parries with his strike (T-044), like a person: no key of its own.
	Intent.bPrimary = Intent.bPrimary || (bParryNow && Side == EJamSide::Hero);
	Intent.bParry = bParryNow && Side == EJamSide::Boss;
	Intent.bDash = ThreatCount > 0 && !bParryNow;
	// The slash flies along the aim, which is at the boss: only his distance decides.
	const bool bFull = Me.Ultimate.Meter >= 1.f && Fight.Ledger.Has(Side, EJamMechanic::Ultimate);
	Intent.bUltimate = bFull && FVector2D::Distance(Me.Pos, Target.Pos) <= Balance.UltimateRange + Target.Radius;
	if (DodgeCircles(Fight, Side, Balance, Intent))
	{
		// The circles come first: they hurt the most.
	}
	else if (StepOffTheLine(Fight, Side, Balance, Intent))
	{
		// His Ultimate next: the long wind-up leaves time to walk.
	}
	else if (IsUnderTheBlade(Fight, Side, Balance))
	{
		// A slow blade is walked out of; the dash is for the one that is about to land.
		Intent.Move = -Intent.Aim.Dir;
		Intent.bDash = Target.Katana.HitTick - Fight.Tick <= JamSim::Ticks(DodgeSeconds);
	}
	else if (bHasKatana && Fight.Ledger.Has(Side, EJamMechanic::Dash) && Side == EJamSide::Hero && JamBossRules::IsBusy(Fight))
	{
		// The boss stands after his attack: this is when the katana pays. Only with the dash in hand: next to him it is
		// the one way through a volley.
		Intent.Move = Intent.Aim.Dir;
	}
	return Intent;
}

int32 JamBotRules::Choose(const FJamFight& Fight, EJamStealPolicy Policy)
{
	const FJamStealState& Steal = Fight.Steal;
	if (Fight.Phase != EJamPhase::Steal || Fight.Tick < Steal.OpenTick)
	{
		return -1;
	}
	const int32 Last = Steal.Offer.Num() - 1;
	// The final cut is no choice: its one key hands everything over.
	if (Steal.bTakesAll)
	{
		return 0;
	}
	switch (Policy)
	{
		case EJamStealPolicy::Right:
			return Last;
		case EJamStealPolicy::Mixed:
			return FMath::Min(Steal.Done % 2, Last);
		default:
			return 0;
	}
}

FJamBotRun JamBotRules::RunFight(const FJamBalanceData& Balance, int32 Seed, EJamStealPolicy Policy, float MaxSeconds)
{
	FJamFight Fight = JamFightRules::Start(Balance, Seed);
	FJamBotRun Run;
	Run.LowestHeroHealth = Fight.Fighters[EJamSide::Hero].Health;
	const int32 MaxSteps = JamSim::Ticks(MaxSeconds);
	int32 Steps = 0;
	while (Steps < MaxSteps && Fight.Phase != EJamPhase::Won && Fight.Phase != EJamPhase::HeroDead)
	{
		FJamIntent Intent = Play(Fight, EJamSide::Hero, Balance);
		Intent.Choice = Choose(Fight, Policy);
		JamFightRules::Play(Fight, Intent, Balance);
		NoteTheCuts(Fight, Run);
		Run.LowestHeroHealth = FMath::Min(Run.LowestHeroHealth, Fight.Fighters[EJamSide::Hero].Health);
		++Steps;
	}
	Run.Outcome = Fight.Phase;
	Run.Seconds = Steps * JamSim::StepDt;
	Run.BossHealthLeft = Fight.Fighters[EJamSide::Boss].Health;
	return Run;
}
