# T-010 Dash

**Block:** B · **Status:** done

## Goal

Space dashes in the move direction (or along the aim when standing still). The fighter is invulnerable during it.

## Scope

- Dash state: direction, time left, cooldown. During the dash the move intent is ignored and the speed is the dash
  speed; the arena clamp still applies.
- Invulnerability window feeds `JamHealthRules::Apply`.
- Numbers (distance, duration, invulnerable time, cooldown) in the balance section.

## Test seam

`JamDashRules` in `Sim`:
- `Direction(MoveInput, AimDir)`: `Jam.Dash.GoesWhereTheFighterMoves`, `Jam.Dash.FallsBackToTheAim`.
- `Start` and `Tick`: `Jam.Dash.CoversTheConfiguredDistance`, `Jam.Dash.CooldownBlocksTheSecondDash`.
- `IsInvulnerable(DashState)`: `Jam.Dash.InvulnerableOnlyInTheWindow`.
- `Jam.Fight.DashThroughAProjectileTakesNoDamage`.

## Done

Playable in PIE. Tests green.
