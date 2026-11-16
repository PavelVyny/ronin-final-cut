# T-104: neon health and Ultimate bars for the full art

Status: done (Mon 16 Nov 2026 UTC, session-6c)

## Asked

Match the approved mockup `Submission/hud_mock_neon.png` (drawn by `Submission/hud_mock_bars.py`) in the HUD's canvas,
no textures: parallelogram bars on a dark plate with faint scanlines, the fill in segments with a thin gap and a bright
top edge, a neon frame with a soft glow, empty segments as a faint outline. The Publisher's bar pink-red with his name
on a slanted tab, the number bright and the whole dim, white cut marks where he steals a feature, and T-053's white
chunk of lost health kept. Ronin's bar cyan with "RONIN 100" over it and a thinner magenta Ultimate bar under it that
pulses when full. Only in the full art: from the first cut on, today's plain bars. Layout math in a pure rules
function with tests.

## Done

- Rules: `JamNeonBarRules` (tests `Jam.Hud.Neon*`, three, red first): `Segments` tiles the bar end to end with equal
  gaps, lit up to the fill (the segment it falls in only in part); `CutMarks` takes the first three steal thresholds
  of `[JamBalance]` as shares; `UltimatePulse` is 0 until the meter is full.
- Drawing: `JamHUDNeon.cpp` (`AJamHUD::DrawNeonBar`, `DrawNeonBoss`, `DrawNeonHero`), the mock's layout as screen shares
  and its pixel sizes in 1080-line pixels (`JamHudPixels`). Every shape, lines included, is canvas triangles: the
  canvas' line items drew a faint colour opaque and the additive blend ignored alpha, which flooded the first try.
- Tiers (the author, after the play-through: two stages, not one): `AJamHUD::bNeonBars` is
  `JamNeonBarRules::Shows`, the hero's own look (`FJamLayers::bHeroArt`, lost at the second cut; test
  `Jam.Hud.NeonBarsLastTwoStages`); otherwise the plain bars draw as
  before, and with the HUD taken (`bBars` false) none.
- The author's play-through, two fixes: one slope (0.6) for every shape of every bar, each point shifted by
  `(BottomY - y) * Slope` from the bar's bottom line (`JamNeonBarRules::Slanted`, test
  `Jam.Hud.NeonFrameAndSegmentsSlantAlike`), so the frame, plate, segments, cut marks and name tab stay parallel; the
  chunk of health just lost is white at alpha 0.9, cut into segments like the fill, and drains as before.
- The cut marks sit where the Publisher steals in this build's balance: 76%, 52% and 27% (the ask said 75, 50 and 25,
  the mock's round numbers). The fourth cut (3%) is not marked.

## Checked

- One fight frame (bot, seed 7, frame 150, 1920x1080) against the mock: `hud/f3/f_00150.png` in the session's
  scratchpad, zooms `f3_boss_zoom.png` and `f3_hero_zoom.png`.
- Not captured: the plain bars after the first cut (the switch is one flag on the layer), the full Ultimate's pulse.
- 323 tests, 0 warnings.
