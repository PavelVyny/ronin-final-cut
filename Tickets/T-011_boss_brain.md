# T-011 Boss brain: chase, memo volley, telegraphed slam

**Block:** B · **Status:** done

## Goal

The Publisher fights back with his own two attacks: a fan of memos (projectiles) and an area slam with a warning
circle. Between attacks he walks at the hero. The hero has health and can die.

## Scope

- The brain is a pure function from what the boss senses to an `FJamIntent` plus boss-only attack orders. It uses the
  seeded stream of the fight.
- Memo volley: N projectiles in a fan around the aim, slow enough to dodge. Marked deflectable for the Parry.
- Slam: a hazard circle at the hero's position that shows for the telegraph time, then damages whoever is inside.
- Attack choice by distance and cooldowns, a recovery pause after each attack (the player's opening).
- Hero health; at zero the fight state becomes `HeroDead` (the screen is T-015).
- View: memo placeholders (flat boxes), a ring decal that fills during the telegraph.

## Test seam

`JamBossRules` in `Sim`:
- `Decide(Sense, Memory, Balance, Stream)`: `Jam.Boss.ChasesWhenFar`, `Jam.Boss.KeepsStillWhileRecovering`,
  `Jam.Boss.SameSeedSameDecisions`, `Jam.Boss.NeverAttacksDuringASteal`.
- `VolleyDirections(AimDir, Count, SpreadDegrees)`: `Jam.Boss.VolleyIsAFanAroundTheAim`.
- `JamHazardRules::Tick` and `Hits`: `Jam.Boss.SlamHurtsOnlyAfterTheTelegraph`,
  `Jam.Boss.SlamMissesAHeroWhoLeftTheCircle`.

## Done

Block B is complete: build and tests green, commit, a clip, three lines in `DEVLOG.md`.
