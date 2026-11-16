# T-021 Five mechanics, four cuts

**Block:** D · **Status:** done

## Goal

The full steal road of the plan: five mechanics, cards with a pair at 80, 60 and 40 percent of the Publisher's
health, and the FINAL NOTE at 20 that takes the two the hero still has.

## Scope

- `JamStealRules::ThresholdCount = 4`, `StealThreshold4` in the balance data, the validation checks that all four go
  down. Game numbers: `0.8 / 0.6 / 0.4 / 0.2`.
- The layers spread over four cuts: effects 1, materials 2, animation 3, model and HUD 4 (the bean at the FINAL NOTE);
  music `StemsLeft=(4,3,2,1,0)`.
- The five mechanics came with T-019 and T-020; the cards and the FINAL NOTE text needed no change.

## Test seam

Existing tests now pin four cuts: `Jam.Steal.LastThresholdTakesTheRest` (three choices of two, the final note takes
the last two at its share), `Jam.Steal.HealthStopsAtTheThreshold` (the fourth floor), `Jam.Balance.StealThresholdsGoDown`
(a fourth threshold out of order), `Jam.Outcome.EveryCutComesBeforeTheStun` (four cards), `Jam.Layers.Ini*` (the
real ini walks four steps to the bean and the metronome). Red first: 12 failing before the change.

## Done

127 `Jam.` tests green, 0 code check warnings. Bot: 60 of 60, median 122 s, closest win at 21 health. Clip
`Clips/t021_first_cut_at_80.mp4`.
