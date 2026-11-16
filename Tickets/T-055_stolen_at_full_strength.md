# T-055 What he stole hits at full strength

**Block:** session 5, fight · **Status:** done

## Goal

The author's playtest of T-051 to T-054 (Sat 14 Nov 18:20): the Publisher is too passive; he rarely uses what he
stole. Remove the handicap on stolen mechanics and shorten his pause.

## Scope

- `[JamBalance]`: `BossStolenDamage` 0.6 to 0.9, `BossStolenCooldown` 2 to 1, `BossStolenSpeed` 0.5 to 0.8,
  `BossRecovery` 1.1 to 0.7 (the author's pick, Sat 18:30).
- If the bot table loses a fight, one step back on the number that costs it, then report.

## Test seam

Numbers only: `Jam.Balance.IniIsCompleteAndValid`, the `Jam.Bot.*` table (60 of 60, every steal order winnable).

## Bot table (20 seeds per policy)

| Policy | T-052 | T-055 |
|---|---|---|
| left | median 73 s | median 84 s, closest win at 42 |
| right | median 73 s | median 81 s, closest win at 28 |
| mixed | median 91 s | median 85 s, closest win at 56 |
| all | 60 of 60, median 79 s, closest win at 32 | 60 of 60, median 81 s (60 to 131), closest win at 28 |

Every steal order still won (`Jam.Bot.EveryStealOrderIsWinnable`). The bot barely feels it: it parries and dashes
perfectly; the real test is a person's run.

## Done

185 `Jam.` tests green, code check without warnings. Clip `Clips/t055_full_strength.mp4` (60 s, bot).
