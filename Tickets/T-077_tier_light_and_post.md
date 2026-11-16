# T-077 Light and post process by tier

**Block:** session 5, polish (track B) · **Status:** done

## Goal

The author, Sun 15 Nov afternoon: the full-art tier must be the most beautiful, so the fall to the lower tiers is
felt. Lumen light and reflections, bloom, cyan and magenta neon lights, emissive surfaces that light the floor, a
slight vignette and grade; no hardware ray tracing (the judges' GPUs may lack it). The lower tiers lose it step by
step.

## Decision

- **Before:** `Config/DefaultEngine.ini` had no renderer section, so the engine defaults ran: no dynamic GI
  (`r.DynamicGlobalIlluminationMethod=0`), screen-space reflections (`r.ReflectionMethod=2`), no mesh distance fields.
  The level is an empty map; the arena view builds a sun, a sky atmosphere and a real-time sky light in code. The only
  per-tier post was the bloom (1.2 with the effects, 0.15 without).
- **Project:** Lumen GI and Lumen reflections, mesh distance fields on (software tracing),
  `r.Lumen.HardwareRayTracing=False`.
- **A rule picks the tier's light** (`JamNeonRules::LightFor`): the full art gets `Neon`, the shape ronin `Flat`,
  greybox (no effects) `Bare`. `JamNeonRules::Bloom` grows a middle step: `Bloom` 1.2 with the full art, `BloomFlat`
  0.15 for the shape ronin, `BloomCut` 0 for greybox.
- **Neon:** the camera's post process turns Lumen GI and reflections on, a 0.5 vignette, saturation and contrast up a
  touch, cool shadows and warm highlights (`AJamCameraRig::SetLight`); sixteen unshadowed cyan and magenta point
  lights stand inside the arena's edge in step with its posts (`AJamLightView`, a new view: the arena view belongs to
  the arena rework). The emissive neon posts, edge and lines feed Lumen, so they light the floor and the bodies.
  After the first look check the floor read grey: the stage's sun is dimmed to a tenth while the full art is on (the
  light view finds it on the arena actor by class), the lamps are 35 cd and Lumen's indirect light is kept at 0.4.
- **Flat:** no GI, screen-space reflections, no lights, neutral grade, a hint of bloom.
- **Bare:** no GI, no reflections, no ambient occlusion, no bloom, neutral grade.

## Test seam

`Jam.Neon.TheLightFallsWithTheTiers` (red first against a stub that always answered `Neon`);
`Jam.Neon.BloomGoesWithTheEffects` and `Jam.Neon.IniIsComplete` still pass unchanged. The look and the frame time
are checked in game.
