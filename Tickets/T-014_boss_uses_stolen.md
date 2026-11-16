# T-014 The boss uses what he stole

**Block:** C · **Status:** done

## Goal

A stolen mechanic appears in the boss's hands for the rest of the fight: he closes in and slashes with the Katana,
throws Shuriken from range (every third explodes), dashes out of the hero's strikes and across the arena.

## Scope

- The brain reads the ledger and adds the owned mechanics to its options. It presses the same buttons through the
  same `FJamIntent`: no boss-only copy of the mechanic code.
- Boss versions differ only by numbers (damage, cooldown multipliers per owner in the balance section) and by a
  telegraph before a katana series so the player can react.
- The hero's weak ball replaces what he lost (T-007).

## Test seam

`JamBossRules::Decide` in `Sim`:
- `Jam.Boss.SlashesWhenCloseAndOwnsKatana`, `Jam.Boss.ThrowsWhenFarAndOwnsShuriken`,
  `Jam.Boss.DashesOutOfAStrikeWhenOwnsDash`, `Jam.Boss.NeverUsesWhatHeDoesNotOwn`.
- `Jam.Fight.BossKatanaDamagesTheHero`: the boss with the Katana in the ledger hurts the hero through `Step`.

## Done

Playable in PIE: each steal visibly changes how the boss fights. Tests green.

## As built

- `JamLoadoutRules::InHandsOf(Boss, Balance)` gives the numbers of a mechanic in his hands: `BossKatanaWindUp`,
  `BossStolenDamage`, `BossStolenSpeed`, `BossStolenCooldown`. The wind-up is the telegraph: his blade sweeps
  through the whole of it, and he stands still while it lasts.
- He rests `BossRecovery` after a finisher, the window for the hero.
- Tests: `Private/Tests/JamStolenTest.cpp` (`Jam.Loadout.StolenIsWeakerInHisHands`, `Jam.Boss.*`,
  `Jam.Fight.BossKatanaDamagesTheHero`).
