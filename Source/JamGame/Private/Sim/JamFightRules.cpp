#include "Sim/JamFightRules.h"

#include "Sim/JamBossRules.h"
#include "Sim/JamDashRules.h"
#include "Sim/JamFightOps.h"
#include "Sim/JamHazardRules.h"
#include "Sim/JamHealRules.h"
#include "Sim/JamKatanaRules.h"
#include "Sim/JamLoadoutRules.h"
#include "Sim/JamMoveRules.h"
#include "Sim/JamOutcomeRules.h"
#include "Sim/JamParryRules.h"
#include "Sim/JamProjectileRules.h"
#include "Sim/JamShurikenRules.h"
#include "Sim/JamStealRules.h"
#include "Sim/JamUltimateRules.h"

namespace
{
// Where the fighters start, as fractions of the arena radius along X: the hero looks at the boss across the centre.
constexpr float HeroStartX = -0.5f;
constexpr float BossStartX = 0.3f;

/** The balance numbers as each side holds them: a stolen mechanic is not the same thing in the Publisher's hands. */
struct FHands
{
	const FJamBalanceData& Hero;
	FJamBalanceData Boss;

	const FJamBalanceData& Of(EJamSide Side) const { return Side == EJamSide::Hero ? Hero : Boss; }
};

FJamFighter MakeFighter(float X, float FacingX, float Radius, float Speed, float Health)
{
	FJamFighter Fighter;
	Fighter.Pos = Fighter.PrevPos = FVector2D(X, 0.0);
	Fighter.Facing = FVector2D(FacingX, 0.0);
	Fighter.Radius = Radius;
	Fighter.Speed = Speed;
	Fighter.Health = Fighter.MaxHealth = Health;
	return Fighter;
}

/** The boss stands through his attacks and the pause after them, whatever his intent says. */
bool IsHeld(const FJamFight& Fight, EJamSide Side)
{
	return Side == EJamSide::Boss && JamBossRules::IsBusy(Fight);
}

void MoveFighter(FJamFight& Fight, EJamSide Side, const FJamIntent& Intent, const FJamBalanceData& Balance)
{
	FJamFighter& Self = Fight.Fighters[Side];
	Self.PrevPos = Self.Pos;
	if (!Intent.Aim.Dir.IsNearlyZero())
	{
		Self.Facing = Intent.Aim.Dir.GetSafeNormal();
	}
	const bool bHeld = IsHeld(Fight, Side);
	const bool bOwnsDash = JamLoadoutRules::ActionFor(EJamButton::Dash, Side, Fight.Ledger) == EJamAction::Dash;
	if (Intent.bDash && bOwnsDash && !bHeld && JamDashRules::CanStart(Self.Dash, Fight.Tick))
	{
		Self.Dash = JamDashRules::Start(JamDashRules::Direction(Intent.Move, Self.Facing), Fight.Tick, Balance);
		JamFightOps::AddEvent(Fight, EJamEvent::Dash, Side, 0, Self.Pos, 0.f);
	}
	FVector2D Delta = FVector2D::ZeroVector;
	if (JamDashRules::IsDashing(Self.Dash, Fight.Tick))
	{
		// A dash keeps its line: the stick is not read until it ends.
		Delta = Self.Dash.Dir * Self.Dash.StepLength;
	}
	else if (!bHeld)
	{
		Delta = JamMoveRules::Velocity(Intent.Move, Self.Speed) * JamSim::StepDt;
	}
	// A fighter that stands is never nudged, whoever walks into it.
	if (!Delta.IsNearlyZero())
	{
		JamFightOps::Shift(Fight, Side, Delta, Balance.ArenaRadius);
	}
}

void AdvanceProjectiles(FJamFight& Fight, const FHands& Hands)
{
	for (int32 Index = Fight.Projectiles.Num() - 1; Index >= 0; --Index)
	{
		FJamProjectile& Flying = Fight.Projectiles[Index];
		Flying.PrevPos = Flying.Pos;
		Flying.Pos += Flying.Vel * JamSim::StepDt;
		const EJamSide TargetSide = JamSim::Other(Flying.Owner);
		const FJamFighter& Target = Fight.Fighters[TargetSide];
		// A dashing fighter is not there for a projectile: it flies through him and on.
		const bool bThere = !JamDashRules::IsInvulnerable(Target.Dash, Fight.Tick);
		const bool bHit =
			bThere && JamProjectileRules::SweepHitsCircle(Flying.PrevPos, Flying.Pos, Flying.Radius, Target.Pos, Target.Radius);
		// An open parry reaches ParryMargin past the body (T-060): it catches what the body alone would not.
		const bool bCaught = bThere && JamParryRules::IsActive(Target.Parry, Fight.Tick) &&
			JamProjectileRules::SweepHitsCircle(Flying.PrevPos, Flying.Pos, Flying.Radius + Flying.ParryMargin, Target.Pos, Target.Radius);
		if (bCaught)
		{
			// The returned projectile keeps the numbers of the side that parried.
			const FVector2D FormerOwnerPos = Fight.Fighters[Flying.Owner].Pos;
			JamParryRules::Deflect(Flying, FormerOwnerPos, Fight.Tick, Hands.Of(TargetSide));
			// T-059: the hero's good parry is ready again at once; the Publisher's stance keeps its cooldown (T-038).
			if (TargetSide == EJamSide::Hero)
			{
				JamParryRules::Rearm(Fight.Fighters[TargetSide].Parry, Fight.Tick);
			}
			JamFightOps::AddEvent(Fight, EJamEvent::Deflected, TargetSide, Flying.Id, Flying.Pos, 0.f);
			continue;
		}
		if (!bHit && !JamProjectileRules::IsGone(Flying, Fight.Tick, Hands.Hero.ArenaRadius))
		{
			continue;
		}
		const FJamProjectile Done = Flying;
		Fight.Projectiles.RemoveAtSwap(Index);
		if (Done.bExplosive)
		{
			JamShurikenRules::Explode(Fight, Done, Hands.Of(Done.Owner));
		}
		else if (bHit)
		{
			JamFightOps::Hurt(Fight, TargetSide, Done.Damage, Done.Pos, Done.Id);
		}
		else
		{
			JamFightOps::AddEvent(Fight, EJamEvent::ProjectileGone, Done.Owner, Done.Id, Done.Pos, 0.f);
		}
	}
}

void ShootBall(FJamFight& Fight, EJamSide Side, const FJamBalanceData& Balance)
{
	FJamFighter& Self = Fight.Fighters[Side];
	if (Fight.Tick < Self.BallReadyTick)
	{
		return;
	}
	Self.BallReadyTick = Fight.Tick + JamSim::Ticks(Balance.BallCooldown);
	FJamShot Ball;
	Ball.Kind = EJamProjectile::Ball;
	Ball.Speed = Balance.BallSpeed;
	Ball.Radius = Balance.BallRadius;
	Ball.Damage = Balance.BallDamage;
	Ball.Life = Balance.BallLife;
	JamFightOps::Shoot(Fight, Side, Self.Facing, Ball);
}

/** A held button does whatever the fighter owns for it right now: the same code for the hero and for the boss. */
void UseButton(FJamFight& Fight, EJamSide Side, EJamButton Button, const FJamBalanceData& Balance)
{
	switch (JamLoadoutRules::ActionFor(Button, Side, Fight.Ledger))
	{
		case EJamAction::Katana:
			JamKatanaRules::Press(Fight, Side, Balance);
			break;
		case EJamAction::Shuriken:
			JamShurikenRules::Throw(Fight, Side, Balance);
			break;
		case EJamAction::Ball:
			ShootBall(Fight, Side, Balance);
			break;
		default:
			break;
	}
}

void UseButtons(FJamFight& Fight, EJamSide Side, const FJamIntent& Intent, const FJamBalanceData& Balance)
{
	if (IsHeld(Fight, Side))
	{
		return;
	}
	FJamFighter& Self = Fight.Fighters[Side];
	const bool bOwnsParry = JamLoadoutRules::ActionFor(EJamButton::Parry, Side, Fight.Ledger) == EJamAction::Parry;
	const bool bCanParry = bOwnsParry && JamParryRules::CanStart(Self.Parry, Fight.Tick);
	// T-044: the hero's strike parries a shot about to land; otherwise it is the strike. bParry stays for the Publisher's
	// brain: his Parry is a stance raised on purpose (T-038), never a side effect of his blade.
	// T-057: a strike a little too early, while the shot is still on its way, is held and parries it once it comes in.
	const bool bHero = Side == EJamSide::Hero;
	const bool bShotNow = bHero && bCanParry && JamParryRules::ShotInReach(Fight, Side, Balance);
	if (bHero && Intent.bPrimary && bCanParry && !bShotNow &&
		JamParryRules::ShotWithin(Fight, Side, Balance.ParryBuffer + Balance.ParryWindow))
	{
		Self.Parry.HeldUntilTick = Fight.Tick + JamSim::Ticks(Balance.ParryBuffer);
	}
	const bool bHeld = bHero && Fight.Tick < Self.Parry.HeldUntilTick;
	const bool bStrikeParries = bShotNow && (Intent.bPrimary || bHeld);
	if ((Intent.bParry || bStrikeParries) && bCanParry)
	{
		const float Raise = Side == EJamSide::Boss ? Balance.BossParryRaise : 0.f;
		Self.Parry = JamParryRules::Start(Fight.Tick, Balance, Raise);
		JamFightOps::AddEvent(Fight, EJamEvent::ParryStarted, Side, 0, Self.Pos, 0.f);
	}
	if (Intent.bPrimary && !bStrikeParries && !bHeld)
	{
		UseButton(Fight, Side, EJamButton::Primary, Balance);
	}
	if (Intent.bSecondary)
	{
		UseButton(Fight, Side, EJamButton::Secondary, Balance);
	}
	if (Intent.bUltimate)
	{
		JamUltimateRules::Press(Fight, Side, Balance);
	}
}

/** A step of the open card: the fighters stand, only the card's clock runs. */
void WaitForTheChoice(FJamFight& Fight, int32 Choice, const FJamBalanceData& Balance)
{
	for (FJamFighter& Fighter : Fight.Fighters.Items)
	{
		Fighter.PrevPos = Fighter.Pos;
	}
	++Fight.Tick;
	JamStealRules::Advance(Fight, Choice, Balance);
}
} // namespace

FJamFight JamFightRules::Start(const FJamBalanceData& Balance, int32 Seed)
{
	FJamFight Fight;
	Fight.Stream.Initialize(Seed);
	Fight.Fighters[EJamSide::Hero] =
		MakeFighter(Balance.ArenaRadius * HeroStartX, 1.f, Balance.HeroRadius, Balance.HeroSpeed, Balance.HeroHealth);
	Fight.Fighters[EJamSide::Boss] =
		MakeFighter(Balance.ArenaRadius * BossStartX, -1.f, Balance.BossRadius, Balance.BossSpeed, Balance.BossHealth);
	// The boss waits a moment before his first move: the player gets a look at the arena.
	Fight.Boss.BusyUntilTick = JamSim::Ticks(Balance.BossRecovery);
	Fight.Steal.Floor = JamStealRules::ThresholdHealth(0, Balance);
	Fight.HeroCutDamageScale = Balance.HeroCutDamageScale;
	Fight.HeroDamageScale = Balance.HeroDamageScale;
	Fight.BossDamageScale = Balance.BossDamageScale;
	Fight.NextHealTick = JamSim::Ticks(Balance.HealDropInterval);
	return Fight;
}

void JamFightRules::Step(FJamFight& Fight, const FJamIntents& Intents, const FJamBalanceData& Balance)
{
	Fight.Events.Reset();
	if (Fight.Phase == EJamPhase::HeroDead || Fight.Phase == EJamPhase::Won)
	{
		// The fight is over: time stands still until somebody starts it again.
		return;
	}
	if (Fight.Phase == EJamPhase::Steal)
	{
		WaitForTheChoice(Fight, Intents[EJamSide::Hero].Choice, Balance);
		return;
	}
	const FHands Hands{ Balance, JamLoadoutRules::InHandsOf(EJamSide::Boss, Balance) };
	for (const EJamSide Side : { EJamSide::Hero, EJamSide::Boss })
	{
		MoveFighter(Fight, Side, Intents[Side], Hands.Of(Side));
	}
	// Projectiles fly before new ones are shot, so a new one is first seen at the muzzle.
	AdvanceProjectiles(Fight, Hands);
	JamHazardRules::Advance(Fight);
	for (const EJamSide Side : { EJamSide::Hero, EJamSide::Boss })
	{
		UseButtons(Fight, Side, Intents[Side], Hands.Of(Side));
	}
	JamBossRules::Act(Fight, Intents[EJamSide::Boss], Balance);
	for (const EJamSide Side : { EJamSide::Hero, EJamSide::Boss })
	{
		JamKatanaRules::Land(Fight, Side, Hands.Of(Side));
	}
	// The meters fill before the slashes land: a slash does not pay for the next one.
	JamUltimateRules::Charge(Fight, Balance);
	for (const EJamSide Side : { EJamSide::Hero, EJamSide::Boss })
	{
		JamUltimateRules::Land(Fight, Side, Hands.Of(Side));
	}
	// After every blow of the step: a hit on the Publisher may knock a heal out.
	JamHealRules::Drop(Fight, Balance);
	JamHealRules::Collect(Fight, Balance);
	JamOutcomeRules::Settle(Fight, Balance);
	++Fight.Tick;
}

void JamFightRules::Play(FJamFight& Fight, const FJamIntent& HeroIntent, const FJamBalanceData& Balance)
{
	FJamIntents Intents;
	Intents[EJamSide::Hero] = HeroIntent;
	Intents[EJamSide::Boss] = JamBossRules::Decide(Fight, Balance);
	Step(Fight, Intents, Balance);
}
