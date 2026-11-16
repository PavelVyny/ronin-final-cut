# T-041 The heavy Publisher

**Block:** session 4 · **Status:** done (numbers; the model waits for T-026)

## Goal

The author's new brief (Sat 14 Nov): the Publisher is a heavy fat-cat corporate boss (concept `r2_boss_c3b`): slow
and clumsy, every blow he lands is very strong. Option taken in chat: the heavy profile, bot table rerun.

## Scope

One block of `[JamBalance]`, nothing in code:

| Number | Before | After |
|---|---|---|
| BossSpeed | 330 | 260 |
| BossRadius | 70 | 90 |
| BossKatanaWindUp | 0.45 s | 0.6 s |
| SlamTelegraph | 0.9 s | 1.1 s |
| SlamDamage | 24 | 34 |
| BossRecovery (his stop after an attack) | 0.9 s | 1.1 s |

Health stays 4800 (Q9).

## Test seam

`Jam.Balance.ThePublisherIsHeavy` reads the shipped numbers against the hero's: slower than half the hero, more than
twice his width, a slam wound up for a second, a sword lifted in 0.55 s or more, a slam worth a third of his health.
Red first on the old numbers (5 of 5 checks failing).

## Bot table (20 seeds per policy)

| Policy | Before (T-031) | Heavy |
|---|---|---|
| left | median 153 s | median 159 s (121 to 242), closest win at 76 health |
| right | median 151 s | median 157 s (117 to 215), closest win at 47 health |
| mixed | median 141 s | median 142 s (120 to 242), closest win at 47 health |
| all | 60 of 60, median 142 s, closest win at 23 | 60 of 60, median 151 s, closest win at 47 |

All 20 steal orders still won. The bot finds him a little easier (it dodges a slow, long-telegraphed slam well); a
person's fight is not measured.

## Done

154 `Jam.` tests green. Clip `Clips/t041_heavy_publisher.mp4` (20 s, bot).
