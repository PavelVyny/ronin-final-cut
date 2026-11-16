# T-007 Mechanics with an owner, the ball fallback

**Block:** B · **Status:** done

## Goal

Every mechanic has an owner, the hero or the boss, and runs the same code for both. A button whose mechanic is gone
fires the weak ball, so the player always has an attack.

## Scope

- `EJamMechanic` (Katana, Shuriken, Dash; Parry and Ultimate reserved) and `FJamLedger`: who owns what.
- Per fighter, per mechanic runtime state (cooldown, combo step, throw counter) in one array, so a change of owner
  needs no copying of code or state.
- Buttons: primary (LMB), secondary (RMB), dash (Space). The action of a button is resolved through the ledger.
- The bean layer stays underneath: with an empty ledger the hero is the block A bean.

## Test seam

`JamLoadoutRules` in `Sim`:
- `ActionFor(Button, Fighter, Ledger)`: `Jam.Loadout.HeroStartsWithEverything`,
  `Jam.Loadout.StolenAttackButtonFiresTheBall`, `Jam.Loadout.StolenDashDoesNothing`,
  `Jam.Loadout.BossOwnsWhatWasGiven`.
- `Give(Ledger, Mechanic, NewOwner)`: `Jam.Loadout.GiveMovesExactlyOneMechanic`.

## Done

Tests green. A debug key (F5) moves the next mechanic to the boss so the fallback can be seen in PIE.
