# T-015 Death, retry from the last steal, win and loss titles

**Block:** C · **Status:** done

## Goal

The fight has an end. Hero dead: "SHIPPED AS PROOF OF CONCEPT" and Retry from the last steal. Boss at zero: he is
stunned, everything returns to the hero, the next hit is the final blow, then "NEON RONIN: DIRECTOR'S CUT".

## Scope

- A checkpoint is a copy of the fight state taken when a steal resolves (and at the start): boss health at the
  threshold, hero at full health, the ledger as after the steal, no projectiles or hazards.
- Retry (Enter or R on the loss screen) restores the checkpoint with a fresh seed.
- Prototype finisher: at zero the boss enters `Stunned`, the ledger returns everything to the hero, any hero hit ends
  the fight as `Won`. The Ultimate replaces "any hit" in T-022.
- Titles as canvas text; Restart from the beginning on the win screen.

## Test seam

- `JamCheckpointRules::Capture(Fight)` and `Restore`: `Jam.Retry.RestoresBossHealthAtTheThreshold`,
  `Jam.Retry.HeroIsHealedAndKeepsOnlyUnstolenMechanics`, `Jam.Retry.ClearsProjectilesAndHazards`.
- `JamOutcomeRules::Phase(Fight)`: `Jam.Outcome.HeroAtZeroIsALoss`, `Jam.Outcome.BossAtZeroIsStunnedNotDead`,
  `Jam.Outcome.EverythingReturnsOnStun`, `Jam.Outcome.HitOnStunnedBossWins`.

## Done

Block C is complete: the fight runs from the first hit to a title in PIE. Build and tests green, commit, a clip,
three lines in `DEVLOG.md`.

## As built

- `JamCheckpointRules::Capture(Fight, Balance)` runs at the start and each time a steal resolves; `Restore(Saved,
  Seed)` gives the same fight with new luck.
- `JamOutcomeRules::Next(Phase, HeroHealth, BossHealth, StealFloor, bBossWasHit)` is the phase table; the stun
  returns every mechanic and clears the stage.
- Keys: R or Enter answers an ending (retry after a loss, play again after a win) and does nothing mid-fight.
  Debug: F5 skips to the next cut, F6 restarts the fight.
- Tests: `Private/Tests/JamOutcomeTest.cpp` (`Jam.Outcome.NextPhaseTable`, `BossAtZeroIsStunnedNotDead`,
  `EverythingReturnsOnTheStun`, `AHitOnTheStunnedBossWins`, `EveryCutComesBeforeTheStun`),
  `Private/Tests/JamRetryTest.cpp` (`Jam.Retry.*`, with `NewSeedNewLuck`).
