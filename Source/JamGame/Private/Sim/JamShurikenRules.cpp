#include "Sim/JamShurikenRules.h"

#include "Sim/JamFightOps.h"

bool JamShurikenRules::IsExplosive(int32 ThrowIndex, int32 Every)
{
	return Every >= 1 && ThrowIndex % Every == 0;
}

bool JamShurikenRules::ExplosionHits(const FVector2D& Center, float Radius, const FVector2D& TargetPos, float TargetRadius)
{
	return FVector2D::DistSquared(Center, TargetPos) <= FMath::Square(static_cast<double>(Radius) + TargetRadius);
}

void JamShurikenRules::Throw(FJamFight& Fight, EJamSide Side, const FJamBalanceData& Balance)
{
	FJamFighter& Self = Fight.Fighters[Side];
	if (Fight.Tick < Self.Shuriken.ReadyTick)
	{
		return;
	}
	Self.Shuriken.ReadyTick = Fight.Tick + JamSim::Ticks(Balance.ShurikenCooldown);
	++Self.Shuriken.Throws;
	FJamShot Shot;
	Shot.Kind = EJamProjectile::Shuriken;
	Shot.Speed = Balance.ShurikenSpeed;
	Shot.Radius = Balance.ShurikenRadius;
	Shot.Damage = Balance.ShurikenDamage;
	Shot.Life = Balance.ShurikenLife;
	// T-060: his stolen shurikens never explode, and the hero's parry catches them from a little further out.
	const bool bHis = Side == EJamSide::Boss;
	Shot.bExplosive = !bHis && IsExplosive(Self.Shuriken.Throws, FMath::RoundToInt32(Balance.ShurikenExplosiveEvery));
	Shot.ParryMargin = bHis ? Balance.BossShurikenParryMargin : 0.f;
	JamFightOps::Shoot(Fight, Side, Self.Facing, Shot);
}

void JamShurikenRules::Explode(FJamFight& Fight, const FJamProjectile& Shuriken, const FJamBalanceData& Balance)
{
	JamFightOps::AddEvent(Fight, EJamEvent::Explosion, Shuriken.Owner, Shuriken.Id, Shuriken.Pos, Balance.ShurikenExplosionRadius);
	const EJamSide TargetSide = JamSim::Other(Shuriken.Owner);
	const FJamFighter& Target = Fight.Fighters[TargetSide];
	if (ExplosionHits(Shuriken.Pos, Balance.ShurikenExplosionRadius, Target.Pos, Target.Radius))
	{
		JamFightOps::Hurt(Fight, TargetSide, Balance.ShurikenExplosionDamage, Target.Pos, Shuriken.Id);
	}
}
