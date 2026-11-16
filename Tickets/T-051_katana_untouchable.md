# T-051 The Katana is untouchable

**Block:** session 5 · **Status:** done

## Goal

The author's playtest of `session-5c` (Sat 14 Nov 16:30): once the Katana goes, the last phases are chipping at a
tough boss with shurikens, and with Shuriken and Dash left, giving away the Shuriken leaves no way to deal damage.
New rule: a choice card never offers the Katana; the pairs come from the other four, so the Katana is always the
last one left. Answers in chat: the FINAL NOTE keeps taking the rest (the Katana with it, the short ball stretch,
then the stun gives everything back); the bot floor goes from 60 to 50 s instead of raising the boss health.

## Scope

- `JamStealRules::Offer` leaves the Katana out of a choice card; the takes-all card is unchanged.
- The bot order table counts orders over the four card mechanics: 20 orders become 12.

## Test seam

`Jam.Steal.TheKatanaIsNeverOnAChoiceCard` (red first: seed 0, card 0 offered it). Changed with the rule:
`Jam.Steal.SameSeedSameOffer` and `Jam.Retry.NewSeedNewLuck` (four left options, not five),
`Jam.Steal.LastThresholdTakesTheRest` (the Katana is one of the two left), `Jam.Bot.EveryStealOrderIsWinnable`
(12 orders), `Jam.Bot.FightLengthIsInRange` floor 50 s.

## Bot table (20 seeds per policy)

| Policy | T-050 | T-051 |
|---|---|---|
| left | median 104 s | median 78 s |
| right | median 83 s | median 67 s |
| mixed | median 80 s | median 81 s |
| all | 60 of 60, median 83 s, closest win at 23 | 60 of 60, median 78 s (60 to 130), closest win at 32 |

Every one of the 12 orders won (2 to 8 runs each).

## Done

175 `Jam.` tests green, code check without warnings. Clip `Clips/t051_katana_kept.mp4` (80 s, bot): cards
Dash/Parry, Parry/Ultimate, Shuriken/Ultimate; the Katana stays on LMB.
