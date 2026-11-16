# T-103: the slate floor's relief

Status: done (Mon 16 Nov 2026 UTC, session-6c)

## Asked

The slate floor reads flat, most of all in the main menu where the camera is low. Keep the glowing cracks between the
stones and the puddles. Material only, no new meshes: a stronger normal, darker cavities, a parallax from the height
map (subtle from the top-down fight camera), and in the menu only a low grazing light, if it leaves the fighters' look.

## Done

- `M_JamArenaFloor` (`Tools/Editor/create_arena_assets.py`, rebuilt with `JAM_IMPORT_ONLY=Floor`):
  - `NormalStrength` 2.5: the normal's RG scaled, then renormalised; puddles still fade it to flat water.
  - `CavityDarken` 0.6: the base colour darkened by up to 60% where the height map is low (0.25 and below a full gap,
    0.6 and above a stone's top), so the stones read as blocks. The cracks' glow is the puddles' reflection, untouched.
  - `ParallaxHeight` 0.02 (tile units, 700 cm tile): a world-space offset along the camera vector by the height map's
    distance from 0.5. The floor tiles in world space, so the mesh's tangents are not used. Almost no shift from the
    fight camera, most at the menu's low one.
- The menu's rim light (`AJamArenaView::RimLight`): a directional light, 6 lux, cool white, 7 degrees above the
  horizon from the skyline's side, 40 degrees off the camera's axis, no shadows. It is on lighting channel 1, which
  only the slate top takes, so the fighters and props keep their light. It shows with the menu's backdrop, and its
  forward shading priority is below the sun's.
- `-JamCapture` also saves the menu's frames (the menu path skipped the capture), for a before/after at the same frame.

## Checked

- Before (Session6d) and after, the menu at 3440x1440 and the full-art fight at frame 150 of the bot's seed-7 run.
  Paths are in the dev log.
- 318 tests, 0 warnings.
