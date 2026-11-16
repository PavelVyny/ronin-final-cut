# T-031 Balance: off the floor

**Block:** G · **Status:** done for the bot (a person's fight is longer and not measured yet)

## Goal

The bot's median sat at 122 s against the 120 s floor of `Jam.Bot.FightLengthIsInRange`: any unrelated change could
drop the game under it. Lift the fight away from the floor without losing a steal order.

## Scope

- One balance change, one line in `[JamBalance]`: `BossHealth` 4000 to 4800. The cuts are fractions of his health,
  so all four still come at 80, 60, 40 and 20 percent.

## Test seam

New `Jam.Bot.EveryPolicyLeavesRoomAboveTheFloor`: each way of answering the cards (left, right, mixed) wins all 20
seeds and keeps its median at least 15 s above the floor. Red first on the old numbers (medians 133, 122, 118).

## Bot table (20 seeds per policy, numbers of the game)

| Policy | Before (4000) | After (4800) | Closest win after |
|---|---|---|---|
| left | 20 of 20, median 133 s | 20 of 20, median 153 s (117 to 214) | 23 health |
| right | 20 of 20, median 122 s | 20 of 20, median 151 s (114 to 212) | 34 health |
| mixed | 20 of 20, median 118 s | 20 of 20, median 141 s (114 to 214) | 42 health |
| all | 60 of 60, median 122 s | 60 of 60, median 142 s | 23 health |

`Jam.Bot.EveryStealOrderIsWinnable`: all 20 orders of the first two cards won.

## Done

143 `Jam.` tests green, 0 code check warnings. Clip `Clips/t031_boss_health_4800.mp4` (20 s, bot).
