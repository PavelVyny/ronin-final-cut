# T-012 Steals at health thresholds

**Block:** C · **Status:** done

## Goal

At 75 and 50 percent of the boss's health the Publisher grabs two random mechanics the hero still owns and the
player gives one away. At 25 percent he takes the rest without a choice.

## Scope

- Thresholds and their kind (pair or take-all) in the balance section; the five-mechanic set of Friday is a change
  of data only.
- The boss's health stops at a threshold: damage beyond it is discarded and the fight enters the `Steal` phase with
  an offer. A resolved steal moves the mechanic in the ledger and bumps the steal counter.
- With one mechanic left a pair threshold offers what there is; with none it is skipped.
- A step event `StealResolved(Index, Mechanic)`: the hook for the cosmetic layers and music stems of block E.

## Test seam

`JamStealRules` in `Sim`:
- `ClampToThreshold(HealthBefore, HealthAfter, Thresholds, StealsDone)`: `Jam.Steal.HealthStopsAtTheThreshold`,
  `Jam.Steal.OneBigHitTriggersOnlyOneSteal`.
- `Offer(Ledger, Kind, Stream)`: `Jam.Steal.OffersTwoDifferentHeroMechanics`, `Jam.Steal.SameSeedSameOffer`,
  `Jam.Steal.LastThresholdTakesTheRest`.
- `Resolve(Fight, Choice)`: `Jam.Steal.ChosenMechanicGoesToTheBoss`, `Jam.Steal.TheOtherOneStays`.

## Done

Tests green; with the debug HUD the ledger changes at the thresholds in PIE.

## As built

- The boss's health has a floor (`FJamStealState::Floor`, the health of the next threshold): `JamFightOps::Hurt`
  discards the damage below it, `JamOutcomeRules::Settle` opens the card at the end of the step that reached it.
- Thresholds are `StealThreshold1..3` in `[JamBalance]`; the last card takes all that is left (`bTakesAll`).
- Events `StealOpened`, `StealResolved`, `Stunned` are the hooks for the cosmetic layers and the music.
- Tests: `Jam.Steal.*` in `Private/Tests/JamStealTest.cpp`, `Jam.Balance.StealThresholdsGoDown`.
