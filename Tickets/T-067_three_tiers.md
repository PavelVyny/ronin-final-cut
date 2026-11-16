# T-067 Three tiers: full art, the shape ronin, greybox

**Block:** session 5, polish · **Status:** done

## Goal

The author, Sun 15 Nov 02:40 (grill answers): the fall from the full version to medium and then to greybox should
read better. Full: the 3D hero with the rich effects; after the first cut the hero goes back to the model of before
the 3D models (the shape ronin) with today's effects; after the second cut the grey world with bare effects. His
warnings stay readable in every tier. (Plan step 3.)

## Decision

- **A sixth layer, the full art** (`FJamLayers::bArt`, `[JamLayers] ArtLostAt=1`): the 3D hero, his afterimages and
  (T-069) the rich effects. Losing it is a visible step down, so it counts in `Total`. The plain effects now go at
  the second cut with the materials (`EffectsLostAt=2`), so the medium tier keeps the glow, the sweep and the trail.
- **The shape ronin is back** as the middle look, as it was before T-061: body, head, hat, blade, its bob, lean and
  arm swing, its T-pose. It lives in `JamFighterRonin.cpp` (the view had grown past 600 lines).
- The road of cuts: 0 the 3D hero; 1 the coloured shape ronin; 2 the grey world, the grey ronin, bare effects;
  3 the T-pose; 4 the bean, the HUD gone. The stun and the win give everything back.

## Test seam

`Jam.Layers.TheArtGoesWithTheFirstCut` (red first; the two ini tests also went red until the line was added). The
test helper `OneLayerPerCut` now loses the art with its first cut, so one existing expectation changed: three cuts
take four layers, not three (the author's new tier, not a bent test). Checked in game: every tier, `-JamDemoSkipEvery=8`.
