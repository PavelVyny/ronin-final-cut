#pragma once

#include "Core/JamTypes.h"
#include "CoreMinimal.h"

/** What a projectile is made of when it is shot. */
struct FJamShot
{
	EJamProjectile Kind = EJamProjectile::Ball;
	float Speed = 0.f;
	float Radius = 0.f;
	float Damage = 0.f;
	/** Seconds of flight. */
	float Life = 0.f;
	bool bExplosive = false;
	/** How far past the target's body a parry still catches it, in cm. */
	float ParryMargin = 0.f;
};

/**
 * The few things every mechanic does to the fight: report an event, hurt a fighter, shoot a projectile, shove a body.
 * They are shared so that the hero's katana and the boss's slam take health by the same rule.
 * Tests: Jam.Fight.*, Jam.Shot.*, Jam.Health.BallDamagesTheBoss, Jam.Steal.HealthStopsAtTheThreshold
 */
namespace JamFightOps
{
JAMGAME_API void AddEvent(FJamFight& Fight, EJamEvent Type, EJamSide Side, int32 ProjectileId, const FVector2D& Pos, float Amount);

/**
 * Takes health from a fighter and reports the hit. The boss's health stops at the threshold of the next steal. A
 * fighter inside his dash window takes nothing, and nobody does while the choice card is up: then the result is false.
 * What the hit means for the fight is decided at the end of the step, by JamOutcomeRules.
 */
JAMGAME_API bool Hurt(FJamFight& Fight, EJamSide TargetSide, float Amount, const FVector2D& Pos, int32 ProjectileId = 0);

/** Empties the air and drops the strikes in their wind-up: the moment belongs to the card or to the last blow. */
JAMGAME_API void ClearStage(FJamFight& Fight);

/** A projectile leaves the fighter along Dir. It starts just outside his body, so it is seen leaving him. */
JAMGAME_API void Shoot(FJamFight& Fight, EJamSide Owner, const FVector2D& Dir, const FJamShot& Shot);

/** Moves a fighter by Delta, around the other fighter and inside the arena. */
JAMGAME_API void Shift(FJamFight& Fight, EJamSide Side, const FVector2D& Delta, float ArenaRadius);
} // namespace JamFightOps
