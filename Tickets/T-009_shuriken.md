# T-009 Shuriken

**Block:** B · **Status:** done

## Goal

RMB throws a fast shuriken along the aim. Every third one explodes on impact or at the end of its flight.

## Scope

- A projectile kind on top of T-005 with its own speed, damage and cooldown.
- A per-owner throw counter; the third throw is marked explosive (the view tints it).
- Explosion: area damage with a radius, hurts enemies of the thrower only.

## Test seam

`JamShurikenRules` in `Sim`:
- `IsExplosive(ThrowIndex, Every)`: `Jam.Shuriken.EveryThirdExplodes`.
- `ExplosionHits(Center, Radius, TargetPos, TargetRadius)`: `Jam.Shuriken.ExplosionReachesTheEdgeOfATarget`.
- `Jam.Fight.ExplosiveShurikenDamagesInRadius`, `Jam.Fight.CounterBelongsToTheThrower` (the boss's throws do not
  advance the hero's counter).

## Done

Playable in PIE. Tests green.
