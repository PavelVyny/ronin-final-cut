#include "Sim/JamFightOps.h"

#include "Sim/JamDashRules.h"
#include "Sim/JamHealthRules.h"
#include "Sim/JamMoveRules.h"
#include "Sim/JamStealRules.h"

void JamFightOps::AddEvent(FJamFight& Fight, EJamEvent Type, EJamSide Side, int32 ProjectileId, const FVector2D& Pos, float Amount)
{
	FJamEvent Event;
	Event.Type = Type;
	Event.Side = Side;
	Event.ProjectileId = ProjectileId;
	Event.Pos = Pos;
	Event.Amount = Amount;
	Fight.Events.Add(Event);
}

bool JamFightOps::Hurt(FJamFight& Fight, EJamSide TargetSide, float Amount, const FVector2D& Pos, int32 ProjectileId)
{
	FJamFighter& Target = Fight.Fighters[TargetSide];
	// Every hit on the Publisher is the hero's: from the first cut on they all land harder. Every hit on the hero is
	// the Publisher's. The difficulty scales both (T-101).
	if (TargetSide == EJamSide::Boss)
	{
		Amount *= Fight.HeroDamageScale * JamHealthRules::HeroDamageScale(Fight.Steal.Done, Fight.HeroCutDamageScale);
	}
	else
	{
		Amount *= Fight.BossDamageScale;
	}
	// Nothing hurts while the card is up or after the end, and a stunned boss cannot hurt back.
	const bool bLive = Fight.Phase == EJamPhase::Fight || (Fight.Phase == EJamPhase::Stunned && TargetSide == EJamSide::Boss);
	const bool bInvulnerable = !bLive || JamDashRules::IsInvulnerable(Target.Dash, Fight.Tick);
	// The boss's health waits at the threshold of the next steal: one big hit never skips a cut.
	const float Floor = TargetSide == EJamSide::Boss ? Fight.Steal.Floor : 0.f;
	const float After = JamStealRules::ClampToThreshold(JamHealthRules::Apply(Target.Health, Amount, bInvulnerable), Floor);
	// A floor above the health he has is not a heal.
	Target.Health = FMath::Min(Target.Health, After);
	if (bInvulnerable)
	{
		return false;
	}
	AddEvent(Fight, EJamEvent::Hit, TargetSide, ProjectileId, Pos, Amount);
	return true;
}

void JamFightOps::ClearStage(FJamFight& Fight)
{
	Fight.Projectiles.Reset();
	Fight.Hazards.Reset();
	// A chain of the Publisher's (T-056) is part of the stage too: it does not go on after the card.
	Fight.Boss.Combo = EJamCombo::None;
	Fight.Boss.Links.Reset();
	for (FJamFighter& Fighter : Fight.Fighters.Items)
	{
		Fighter.Katana.bHitPending = false;
	}
}

void JamFightOps::Shoot(FJamFight& Fight, EJamSide Owner, const FVector2D& Dir, const FJamShot& Shot)
{
	const FJamFighter& Self = Fight.Fighters[Owner];
	FJamProjectile Projectile;
	Projectile.Id = Fight.NextId++;
	Projectile.Kind = Shot.Kind;
	Projectile.Owner = Owner;
	Projectile.Pos = Projectile.PrevPos = Self.Pos + Dir * (Self.Radius + Shot.Radius);
	Projectile.Vel = Dir * Shot.Speed;
	Projectile.Radius = Shot.Radius;
	Projectile.Damage = Shot.Damage;
	Projectile.DieTick = Fight.Tick + JamSim::Ticks(Shot.Life);
	Projectile.bExplosive = Shot.bExplosive;
	Projectile.ParryMargin = Shot.ParryMargin;
	Fight.Projectiles.Add(Projectile);
	AddEvent(Fight, EJamEvent::ProjectileSpawned, Owner, Projectile.Id, Projectile.Pos, 0.f);
}

void JamFightOps::Shift(FJamFight& Fight, EJamSide Side, const FVector2D& Delta, float ArenaRadius)
{
	FJamFighter& Self = Fight.Fighters[Side];
	const FJamFighter& Other = Fight.Fighters[JamSim::Other(Side)];
	const FVector2D Pos = JamMoveRules::PushOut(Self.Pos + Delta, Self.Radius, Other.Pos, Other.Radius);
	Self.Pos = JamMoveRules::ClampToArena(Pos, Self.Radius, ArenaRadius);
}
