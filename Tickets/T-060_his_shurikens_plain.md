# T-060 His stolen shurikens never explode

**Block:** session 5, fight · **Status:** done

## Goal

The author on `session-5i` (Sat 14 Nov 19:50): when the Publisher steals the Shuriken he throws as often as now, but
none of his explode, and they are a little easier to parry with the strike (slower or a larger parry reach, the
agent's call). The hero's own "every third explodes" stays (the author's answer, same evening).

## Decision

A larger parry reach, the speed kept: his shurikens carry `BossShurikenParryMargin=30` (cm). While the parry is open,
one of his that passes within the hero's body plus 30 cm goes back, and the strike's look-ahead (T-044, T-057) sees it
from that far out. What goes back loses the margin. His shurikens were already 0.8 of the hero's speed (T-055).

## Test seam

`Jam.Shuriken.HisStolenOnesNeverExplode` (red first: 2 of 6 exploded, no margin), `Jam.Parry.HisShurikenIsCaughtFromFurtherOut`
(red first: passed by).

## As built

- `JamShurikenRules::Throw`: the boss's shot is never explosive and carries the margin. Not a zero in `[JamBalance]`:
  the validator reads every zero as a missing line.
- `FJamShot::ParryMargin`, `FJamProjectile::ParryMargin`; `AdvanceProjectiles` deflects on the wider sweep while the
  parry is open, `JamParryRules::ShotWithin` looks with it.
- The blast ring under his explosive shurikens (`JamFeedbackRules::ShowsBlastRing`) no longer has anything to show;
  left in place, its test still passes.

## Done

209 `Jam.` tests green, code check without warnings. Bot table after T-059 and T-060: 60 of 60; medians 102 / 97 /
101 s (were 99 / 97 / 102); closest win 26 / 44 / 60 health (were 26 / 40 / 46). A little easier for the bot, not
too easy by its numbers; the bot never spams, so the author's hands are the real check.
