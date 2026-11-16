# T-045 An arrow to the Publisher off screen

**Block:** session 5 · **Status:** done

## Goal

The author's playtest (Sat 14 Nov 14:20): show where the boss is when he is outside the screen.

## Scope

- `JamMarkerRules::EdgeMarker`: hidden while the point is inside the screen less a margin; otherwise on that inset
  edge where the line from the screen centre to the point crosses it, with the angle it points at.
- `AJamHUD::DrawBossMarker`: a red wedge (the colour of his health bar), 34 px, 48 px from the edge, pointing at a
  point 1 m up his body. Drawn in the fight and the stun, not over the card or an ending. `RenderCore` added to the
  module for the canvas triangle.

## Test seam

`Jam.Hud.AnArrowPointsAtTheBossOffScreen` (on screen, right, above, a corner, the margin); red first on a stub (7
checks).

## Done

161 `Jam.` tests green. Clip `Clips/t045_boss_marker.mp4` (30 s, bot): the arrow at the top edge in the first
second, while the Publisher walks in from off screen.
