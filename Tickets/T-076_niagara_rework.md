# T-076 Niagara rework for the full-art tier

**Block:** session 5, polish (track B) · **Status:** done

## Goal

The author, Sun 15 Nov afternoon: the hit effects of the full-art tier still look like a lower tier. Hits want a
bright core flash with a short-lived light and more, longer streaks; slams and blasts a shockwave ring; cash bursts a
sparkle. The Publisher must stay readable (he washed white under the T-069 bursts).

## Decision

Edited in the editor through the MCP's Niagara toolset; the user parameters the code drives are unchanged
(`User.Color`, `User.Scale` on the sparks, `User.Radius` on the dust; `NS_JamUltCharge` and `NS_JamTrail` untouched).

- **`NS_JamHitSparks`:** the streaks are 40 per hit (36 before), live 0.25 to 0.55 s, 3 x 75 cm, fly 420 to 950 cm/s,
  all scaled by the square root of `User.Scale`, so the Ultimate's 2.2 no longer doubles everything; their colour is
  the attacker's neon at 4x (12x before, which bloomed to white). The flash is a small white-hot core (18 cm, 24x,
  0.1 s, 60 cm and 20x before) with a **Niagara light** (radius 220 cm x sqrt(Scale), diffuse 0.3), so a hit lights
  the floor and the bodies for a tenth of a second instead of covering them.
- **`NS_JamSlamDust`:** the embers emitter became a **shockwave ring**: 72 velocity-aligned hot streaks on a ring
  that run out to `User.Radius` in about 0.3 s, flat on the floor; the grey smoke and the debris stay. Explosions use
  the same system, so a blast rings to its radius too.
- **`NS_JamCashBurst`:** 40 sparkles (15 before), smaller, gold-tinted at 18x, glinting fast instead of pulsing.
- **Checked from the game camera** in a demo run (`Saved/Demo/T076_compare/`): the Publisher's body reads through his
  hits, the hits read cyan on him and red on the hero, the slam's ring meets its warning circle.

## Test seam

None in code: asset changes only. The look is checked by eye from the game camera; `Jam.Fx.*` still pass.
