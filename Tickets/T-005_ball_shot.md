# T-005 The weak ball shot and projectiles

**Block:** A · **Status:** done

## Goal

The bean shoots a slow weak ball along the aim. This is the proof-of-concept attack the player always has.

## Scope

- `FJamProjectile` in the sim: kind, owner, position, velocity, radius, damage, lifetime.
- The projectile step: move, hit the first enemy fighter circle, die at the arena edge or at the end of life.
- The ball: a cooldown and a spawn at the muzzle offset. Numbers in the balance section.
- `AJamProjectileView`: a sphere mesh per live projectile, pooled.

## Test seam

- The cooldown is a tick comparison inside `JamFightRules::Step`: `Jam.Shot.CooldownBlocksTheSecondShot`.
- `JamProjectileRules::Advance(Projectile, Dt)` and `Hits(Projectile, Fighter)`:
  `Jam.Projectile.HitsACircleOnItsPath` (swept, so a fast projectile does not tunnel),
  `Jam.Projectile.IgnoresItsOwner`, `Jam.Projectile.DiesOutsideTheArena`.
- `Jam.Fight.BallSpawnsAlongTheAim`: one `Step` with the fire button gives one projectile flying along the aim.

## Done

The ball flies and disappears in PIE in all three cameras. Tests green.
