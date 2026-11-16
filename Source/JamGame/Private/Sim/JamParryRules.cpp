#include "Sim/JamParryRules.h"

bool JamParryRules::CanStart(const FJamParryState& State, int32 Tick)
{
	return Tick >= State.ReadyTick;
}

FJamParryState JamParryRules::Start(int32 Tick, const FJamBalanceData& Balance, float RaiseSeconds)
{
	FJamParryState State;
	State.ActiveFromTick = Tick + (RaiseSeconds > 0.f ? JamSim::Ticks(RaiseSeconds) : 0);
	State.ActiveUntilTick = State.ActiveFromTick + JamSim::Ticks(Balance.ParryWindow);
	// A cooldown shorter than the window would let a held key keep the window open for ever.
	State.ReadyTick = FMath::Max(State.ActiveUntilTick, Tick + JamSim::Ticks(Balance.ParryCooldown));
	return State;
}

bool JamParryRules::IsActive(const FJamParryState& State, int32 Tick)
{
	return Tick >= State.ActiveFromTick && Tick < State.ActiveUntilTick;
}

void JamParryRules::Rearm(FJamParryState& State, int32 Tick)
{
	State.ReadyTick = FMath::Min(State.ReadyTick, Tick);
}

bool JamParryRules::IsRaising(const FJamParryState& State, int32 Tick)
{
	return Tick < State.ActiveFromTick;
}

bool JamParryRules::ShotWithin(const FJamFight& Fight, EJamSide Side, float Seconds)
{
	const FJamFighter& Me = Fight.Fighters[Side];
	for (const FJamProjectile& Flying : Fight.Projectiles)
	{
		if (Flying.Owner == Side)
		{
			continue;
		}
		// Where the shot passes in that time: if that path touches the body, it arrives within it.
		const FVector2D Closest = FMath::ClosestPointOnSegment2D(Me.Pos, Flying.Pos, Flying.Pos + Flying.Vel * Seconds);
		if (FVector2D::Distance(Closest, Me.Pos) <= Me.Radius + Flying.Radius + Flying.ParryMargin)
		{
			return true;
		}
	}
	return false;
}

bool JamParryRules::ShotInReach(const FJamFight& Fight, EJamSide Side, const FJamBalanceData& Balance)
{
	// A parry opened now catches what arrives within one window. The projectiles of this step have already flown when it
	// opens, so a window of N steps guards the next N - 1 flights (T-057: a shot landing on its last step got through).
	return ShotWithin(Fight, Side, (JamSim::Ticks(Balance.ParryWindow) - 1) * JamSim::StepDt);
}

void JamParryRules::Deflect(FJamProjectile& Projectile, const FVector2D& FormerOwnerPos, int32 Tick, const FJamBalanceData& Balance)
{
	FVector2D Dir = (FormerOwnerPos - Projectile.Pos).GetSafeNormal();
	if (Dir.IsNearlyZero())
	{
		Dir = -Projectile.Vel.GetSafeNormal();
	}
	Projectile.Owner = JamSim::Other(Projectile.Owner);
	Projectile.Vel = Dir * Balance.ParryReturnSpeed;
	Projectile.Damage *= Balance.ParryDamageScale;
	Projectile.DieTick = Tick + JamSim::Ticks(Balance.ParryReturnLife);
	// The wider catch was for the side it flew at; what goes back is caught like any other shot.
	Projectile.ParryMargin = 0.f;
}
