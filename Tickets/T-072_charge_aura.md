# T-072 The Ultimate charge is an aura on the hero, the dash loses its bar

**Block:** session 5, Sunday feedback · **Status:** done

## Goal

The author played session-5m, Sun 15 Nov: in the full-art tier the Ultimate charge and the dash look low-poly, plain
bars. In the full-art tier the bars go; the charge becomes a Niagara effect on the hero that grows with the charge
and clearly reads as "ready" when full; the dash keeps only its afterimages. These degrade with the look: one tier
down a simpler version, two tiers down (greybox) the bars come back. The tier choice is a pure rule with a test.

Answer to the question which bars (Sun 15 Nov): the floor shapes and the HUD meter.

## Decision

- **`JamFxRules::ChargeLook(Shown)`**: `Aura` while the full art is on, `Glow` while the plain effects are (the shape
  ronin), `Bars` from greybox on.
- **Aura (full art):** `NS_JamUltCharge` stands at the hero's feet: magenta motes rising round him and a glow on the
  floor. `JamFxRules::Aura(Meter, bOwns, bWindingUp)` sizes it: from 0.35 and 6 motes a second when the meter is
  empty up to 0.7 and 35 nearly full, then a step to 1.0 and 90 when full (and a magenta spark burst off his chest
  the moment it fills), 1.4 and 180 while he winds the cut up. No HUD meter bar, no lane fill, no wind-up disc, no
  spinning ready ring on the floor; the lane's thin aim edges stay, they show where the cut goes. The dash draws no
  floor bar: its afterimages (T-066) carry it.
- **Glow (shape ronin):** a dim magenta disc under him that grows with the meter; when full, the spinning ready ring as
  before; the dash leaves a thin streak, a third of the bar's width. No HUD meter bar.
- **Bars (greybox):** everything as before T-072: the HUD meter, the lane fill, the wind-up disc, the dash bar.
- Fixed on the way: `JamHUDCrack.cpp` (T-071) named a parameter `Frame`, which a new unity grouping made clash with a
  global (C4459).

## Test seam

`Jam.Fx.TheChargeLooksDegradeWithTheCuts`, `Jam.Fx.TheAuraGrowsWithTheChargeAndSaysReady` (red first against
stubs). The look is checked from the game camera.
