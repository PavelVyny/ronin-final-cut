# T-053 Health bars: delayed damage chunk and heal preview

**Block:** session 5 · **Status:** done

## Goal

The author's playtest of `session-5c` (Sat 14 Nov 16:30): the health bars feel instant. Follow the game UI pattern
of the delayed damage chunk. Damage: the lost part turns white instead of vanishing; while damage keeps coming the
white part keeps growing; after a short pause with no new damage (0.5 to 1 s) it drains away. Heal: the gained part
shows at once as a light green preview and the bar fills up to it over a short time. Both bars, a pure rule for the
displayed values, the simulation untouched.

## Scope

- `JamBarRules::Track` (UI, presentation only): fill, chunk top, preview top and the seconds since the last hit, per
  bar. A hit drops the fill at once and keeps the chunk's top; a heal moves the preview at once and fills at
  `HealFill`; the chunk drains at `ChunkDrain` after `ChunkHold` without damage. The first look snaps to the health.
- `[JamBar]` in `DefaultGame.ini`: `ChunkHold=0.6`, `ChunkDrain=0.8` bars/s, `HealFill=0.6` bars/s (a 25 heal on the
  hero's 100 fills in about 0.4 s).
- `AJamHUD` keeps one state per fighter, tracks it every frame with the world's delta and draws back, white chunk,
  green preview, solid fill. When the Publisher takes the HUD the bars go as before.

## Test seam

`Jam.Hud.TheBarStartsAtTheHealth`, `Jam.Hud.DamageLeavesAWhiteChunkThatGrows`, `Jam.Hud.TheChunkDrainsAfterAPause`,
`Jam.Hud.AHealShowsAPreviewAndFillsUp` (red first on an empty stub), `Jam.Hud.TheBarNumbersAreSet` (the game's
numbers in range).

## Done

182 `Jam.` tests green, code check without warnings. Clip `Clips/t053_health_bars.mp4` (45 s, bot): the boss bar's
white chunk grows through a combo (3830 to 3040) and drains about 0.25 s after the hits stop.

## Note for a playtest

A player who keeps hitting without a 0.6 s break keeps the chunk on the bar until the break, as asked. If it reads too
long, `ChunkHold` is one line in `[JamBar]`.
