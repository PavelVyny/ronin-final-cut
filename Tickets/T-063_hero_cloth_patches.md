# T-063 The hero's black cloth without the red and light-blue patches

**Block:** session 5, models · **Status:** done, applied for the author's morning review

## Goal

The author, Sun 15 Nov 00:30: on the black cloth of the hero's back and sides there are red and light-blue patches;
paint them over with the surrounding black cloth in the base colour map. The cyan and magenta neon piping and the
mask stay untouched; check all four sides. The author let it be applied overnight as its own commit (same night).

## Decision

The patches are soft glow halos Meshy baked into the base colour around the piping and under the arms. They are
dimmer than the piping and saturated, cyan or red-to-magenta in hue. `Tools/Art/hero_cloth_fix.py` repaints exactly
those texels of the cloth with the median black of the cloth (keeping 35% of their own shading). The cloth is found
from the skin: faces skinned mostly to the head, neck, hands or feet are protected (6695 of 20572 faces, with a
margin in UV space), so the mask, the face and the hair keep their colours. 17% of the cloth texels changed.

## Check

Flat base-colour renders of all four sides before and after (`Tools/Art/flat_views.py`):
`Art/Models/hero_fixed/textures/cloth_before_after.png` (top row before, bottom after), and a close look from the
game camera. The Meshy original is kept as `base_color_meshy.png`: copying it back over `base_color.png` and running
`Tools/Editor/create_character_materials.py` reverts the change.

## Left as it was

The purple lining inside the front flaps (hue outside the patch range) and the red-brown streak in the hair at the
temples (the head is protected): neither is on the black cloth.
