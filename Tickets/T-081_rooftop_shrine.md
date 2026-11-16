# T-081 The rooftop shrine: the arena of the full art

**Block:** session 5, environment · **Status:** done

## Goal

The author, Sun 15 Nov 11:50: the arena is fine for the degraded tiers but not for the start of the game, when
nothing is cut yet. Build it in detail. Three concepts were made (`Art/Concepts/r6_arena_*`); the author chose A,
the rooftop shrine in a rainy neon city. Grill answers: props from Meshy, time box until about 21:00 UTC, the middle
tier is the bare shrine, rain with a wet floor and splashes where the fighters step.

## Decision

- **Three looks** (`JamArenaRules::LookFor`): the full art is the shrine (slate with the engraved crest, 16 lantern
  pillars, three torii, eight stone lanterns, four bonsai, a railing, the city far below, rain); the first cut leaves
  the bare shrine (slate and pillars); the second cut leaves the engine grid with plain posts, as before. The neon
  edge band stays in both shrine looks and now carries the beat of the music; the floor grid lines are gone.
- **No gate at -X:** the camera looks along +X, a gate on the near edge would hide the hero.
- **Splashes:** one per stride under each fighter (`JamArenaRules::StepSplashes`, hero 110 cm, Publisher 180 cm),
  shrine only; a jump longer than a dash in one frame (a retry) does not splash.
- **Assets** by `Tools/Editor/create_arena_assets.py` (headless): Meshy props on `M_JamArenaProp` (the bright parts
  of the base colour glow: lantern glass and paper), `M_JamArenaFloor` (PBRgen slate in world space, puddles in the
  low spots of the height map, the crest's bronze lines laid in), `M_JamArenaCity`. Loaded by path; a missing asset
  leaves its place empty. Lights and post process stay with Track B (`AJamLightView`).

## Test seam

`Jam.Arena.EachTierHasItsLook`, `Jam.Arena.OneSplashPerStride`, red first (stub rules: 2 failing), then green.
