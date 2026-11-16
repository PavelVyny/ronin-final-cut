# T-047 The boss arrow keeps clear of his health bar

**Block:** session 5 · **Status:** done

## Goal

The author's playtest of `session-5a` (Sat 14 Nov 15:05): with the Publisher off to the north, the edge arrow (T-045)
touched his health bar and read as a triangle growing out of it. Option picked: the arrow stops under the bar.

## Scope

`JamMarkerRules::EdgeMarker` takes a top inset apart from the side margin. The HUD passes the bottom of the bar's
numbers plus a 16 px gap plus half the arrow. The bar and the rest of the HUD do not move.

## Test seam

`Jam.Hud.TheArrowKeepsClearOfTheBossBar` (north, north-east, a point behind the bar, the sides keep their margin);
red first (3 checks). `Jam.Hud.AnArrowPointsAtTheBossOffScreen` calls the new form with the same inset all round.

## Done

163 `Jam.` tests green. Clip `Clips/t047_arrow_gap.mp4` (8 s, bot): the arrow under the bar in the first second.
