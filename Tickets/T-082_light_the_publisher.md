# T-082 A key light on the Publisher, a brighter night, no glow pools

**Block:** session 5, Sunday quick fixes before session-5p · **Status:** done

## Goal

The author on session-5o: the arena is too dark and the Publisher's kung-fu show-off in the intro is barely visible;
light him and lift the exposure a bit. The coloured glow pools under both fighters (red under him, blue under the
hero) look bad: remove them.

## Decision

- `AJamLightView` gets a warm spot light that follows the Publisher (from above and from the camera's side, aimed at
  his chest), only in the full-art tier. 9000 cd washed him white on the first look; 1100 cd keeps his hands and
  shoes readable.
- The night is lifted: the arena sun keeps 25% (was 10%), the 16 lamps 55 cd (was 35), and the camera's exposure bias
  is +0.5 in the full art.
- The point light under each fighter (`Glow` in AJamFighterView, the pools) is gone in every tier.

## Test seam

Presentation only; checked from the game camera in the intro and the fight.
