# T-027 The neon arena and VFX on stand-ins

**Block:** F · **Status:** done on stand-ins (real models and VFX wait for the art, T-025, T-026)

## Goal

The full look has to be worth losing: a neon arena that blooms, so the drop to the engine grid at the second cut
reads as a loss.

## Scope

- `M_JamNeon` (`/Game/Materials`): unlit, all emission, `Color` times `Glow`; made by
  `Tools/Editor/create_neon_material.py` (headless pythonscript commandlet). `JamShapes::Glow` paints with it.
- Arena: posts, floor lines and a new neon band along the edge glow; the lines pulse on every beat of the 120 BPM
  music. The second cut still puts the grid on everything and hides lines and the edge.
- Effects (blades, flashes, warnings, sparks, rings) and projectiles glow; memos stay paper (glow 1).
- `JamNeonRules` (`View`, pure), numbers in `[JamNeon]`: `LineGlow(MusicTime)` (a kick on each beat, an exponential
  fall, never below the base), `Bloom(Layers)` (the camera blooms with the effects layer; the first cut takes the
  bloom with the effects).

## Test seam

`Jam.Neon.LinesPulseOnTheBeat`, `Jam.Neon.BloomGoesWithTheEffects`, `Jam.Neon.IniIsComplete`. Red first: 3 failing
with stub rules; on the way green the beat test caught a rounding epsilon in the code that dimmed the beat.

## Done

142 `Jam.` tests green, 0 code check warnings. Clip `Clips/t027_neon_arena.mp4` (30 s, bot). After the clip the posts
and edge were toned down in the ini (`PostGlow` 4 to 2.5, `EdgeGlow` 6 to 4): at 4 the posts washed out to pastel.
