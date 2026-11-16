# T-043 A shorter fight, a short final stretch, time to think on the card

**Block:** session 5 · **Status:** done

## Goal

The author's playtest (Sat 14 Nov 14:20): health back to 4000; the part after the FINAL NOTE, when the hero has only
the ball, is too long and frustrating; the steal card should give more time to think.

## Scope

`[JamBalance]` only:

| Number | Before | After |
|---|---|---|
| BossHealth | 4800 | 4000 |
| StealThreshold1 to 4 | 0.8, 0.6, 0.4, 0.2 | 0.76, 0.52, 0.27, 0.03 |
| StealChoiceSeconds | 3 | 7 |

The ball-only stretch goes from 960 health (about 67 s of steady fire) to 120 (about 8 s); the four stretches before
it stay about even (960, 960, 1000, 960).

## Test seam

`Jam.Balance.TheFinalStretchIsShort`: 4000 health, the last stretch at most 12 s of steady ball fire and under a
quarter of any other, the card at least 6 s. Red first on the old numbers (4 of 4 checks).

## Bot table (20 seeds per policy)

| Policy | T-041 | T-043 |
|---|---|---|
| left | median 159 s | median 101 s |
| right | median 157 s | median 96 s |
| mixed | median 142 s | median 84 s |
| all | 60 of 60, median 151 s, closest win at 47 | 60 of 60, median 92 s (59 to 187), closest win at 87 |

The bot median floor of `Jam.Bot.FightLengthIsInRange` and `Jam.Bot.EveryPolicyLeavesRoomAboveTheFloor` went from 120
to 60 s on the author's word (a shorter fight was the ask).

## Done

157 `Jam.` tests green. No clip: numbers only, the bot table is the evidence.
