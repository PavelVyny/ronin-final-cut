# T-075 The full-art tier shows its blows with Niagara alone

**Block:** session 5, polish (track B) · **Status:** done

## Goal

The author, Sun 15 Nov afternoon, after session-5n: the hit effects of the full-art tier still look like a lower tier
(small flat sparks, red on the hero). `AJamEffectsView::DrawBurst` drew its plain discs in every tier, under the
Niagara bursts of T-069, so the top tier carried the greybox layer too.

## Decision

- **A rule decides which flat shapes the full art keeps** (`JamFxRules::DrawsPlain`): with the full art on, the hit,
  slam, explosion, deflect and heal discs are off and the Niagara bursts of `JamFxRules::CuesFor` show those events
  alone. The shape ronin and the greybox keep every disc as before.
- **What carries gameplay stays in every tier:** the sweep of a blade (the reach that really hits), the Ultimate's line
  (the lane it cut), the dash, and everything `DrawCircles` draws (telegraph circles, the slam warning, the blast
  rings, the cash on the floor).
- `AJamGameMode::ApplyLayers` hands the effects view `bArt` with the other layers.

## Test seam

`Jam.Fx.TheFullArtLeavesTheBurstsToNiagara` (red first against a stub that kept every disc). The look is checked from
the game camera in the full-art and shape-ronin tiers.
