# T-006 Boss dummy with health, debug HUD

**Block:** A · **Status:** done

## Goal

A big bean stands in the arena: the Publisher. Balls take his health down; a bar on screen shows it.

## Scope

- The second fighter in `FJamFight` with the boss radius and health from the balance section.
- Damage application and the hit event list of a step (the view and later the sound read events, the sim does not
  call them).
- `AJamHUD` (canvas, no assets): boss bar on top, hero health, the active camera name, the build version.

## Test seam

`JamHealthRules` in `Sim`:
- `Apply(Health, Damage, bInvulnerable)`: `Jam.Health.DamageReducesHealth`, `Jam.Health.NeverBelowZero`,
  `Jam.Health.InvulnerableTakesNothing`.
- `Jam.Fight.BallDamagesTheBoss`: a ball fired at the boss in `Step` lowers his health and emits one hit event.

## Done

Block A is complete: build and tests green, commit, a clip, three lines in `DEVLOG.md`.
