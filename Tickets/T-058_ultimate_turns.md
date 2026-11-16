# T-058 The Ultimate turns toward the cursor during its wind-up

**Block:** session 5, fight · **Status:** done

## Goal

The author's playtest: once the Ultimate is pressed its line is locked. The author's answer (Sat 14 Nov 18:30): the
hero's line keeps turning toward the cursor at `UltimateTurnRate=120` degrees per second through the wind-up (about
40 degrees of correction in 0.35 s). The Publisher's line stays locked (T-038).

## Test seam

`Jam.Ultimate.LineTurnsTowardTheAim` (limited per step), `Jam.Ultimate.BossLineStaysLocked`; the T-046 lane follows
the turned line.

## As built

- `UltimateTurnRate=120` in `[JamBalance]`. `JamUltimateRules::TurnToward` (the short way round, at most the step's
  share); `Land` turns the hero's pending line toward his facing every step of the wind-up, the Publisher's stays.
- The T-046 lane already draws the pending line, so it follows the turn.

## Test seam (as built)

`Jam.Ultimate.LineTurnsTowardTheAim`, `Jam.Ultimate.BossLineStaysLocked`. Red first on a no-turn stub (the boss
test also failed there, on its own bug: a fresh fight keeps him in his opening pause, so he never pressed).

## Done

205 `Jam.` tests green, code check without warnings, bot 60 of 60, median 99 s. Clip pending: the game window
stops rendering while covered, so the take saved 20 frames; recorded with the fight-block build.
