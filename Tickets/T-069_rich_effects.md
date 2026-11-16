# T-069 Rich Niagara effects for the full-art tier

**Block:** session 5, polish · **Status:** done

## Goal

The author, Sun 15 Nov 02:40: the full version needs more detailed Niagara effects, so the fall from full to medium
to greybox reads better. Grill answer: Niagara, with the Unreal MCP on. (Plan step 5.)

## Decision

- **Five systems in `/Game/FX`**, built in the editor through the MCP's Niagara toolset by a helper agent:
  `NS_JamHitSparks` (velocity-aligned streaks and a flash, `User.Color`, `User.Scale`), `NS_JamSlamDust` (a ring of
  smoke to `User.Radius`, debris, embers), `NS_JamTrail` (a world-space ribbon, `User.Color`, `User.Width`),
  `NS_JamCashBurst` (tumbling bills and sparkles), `NS_JamEmbers` (cyan and magenta embers drifting over the arena).
  All compile clean; two helper materials (`M_JamFXSoft`, `M_JamFXSolid`).
- **Which event starts what** is a rule (`JamFxRules::CuesFor`): a hit sparks off the body that was hit in the
  attacker's neon, sized by the damage; a slam raises dust to its radius; an explosion sparks yellow and raises dust;
  a deflect sparks white; a taken heal bursts cash; the Ultimate's slash sparks big in magenta. Nothing once the art
  is cut: the medium tier keeps the plain effects, the greybox the bare ones.
- **Trails**: the katana's tip leaves a cyan ribbon while it swings; each shuriken leaves a ribbon in its side's
  colour (kept per shot id, so it never jumps between shots, and let fade when the shot is gone). The embers run
  over the arena while the art is whole and die out with the first cut.
- First in-game look was too strong (the Publisher washed white under big bursts and white smoke): the burst sizes
  are capped lower (1.8, blast 1.5, Ultimate 2.2) and the smoke is grey.

## Test seam

`Jam.Fx.HitsSparkInTheAttackersColour`, `Jam.Fx.SlamsAndBlastsRaiseDustToTheirRadius`,
`Jam.Fx.DeflectsCashAndTheUltimate`, `Jam.Fx.NothingRichOnceTheArtIsCut` (three red first against a stub; the
fourth passes on an empty stub by its nature). The look is checked from the game camera.
