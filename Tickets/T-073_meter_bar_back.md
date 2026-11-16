# T-073 The Ultimate meter bar is back, the aura says only "ready"

**Block:** session 5, Sunday afternoon · **Status:** done

## Goal

The author played session-5n: the charge aura says nothing while the meter fills. Bring back the Ultimate meter as
its own bar under the hero's health, in every tier, as before T-072. The aura under the hero shows only when the
meter is full (and through the wind-up), never earlier. Shape ronin and greybox: the bar always, their ready cue only
when full.

## Decision

- `JamFxRules::Aura` is off until the meter is full; full: size 1, 90 motes a second; the wind-up: 1.4 and 180 (it
  shows through the wind-up even though the meter was spent on it).
- The HUD draws the meter bar in every tier again (the `bMeterBar` switch of T-072 is gone).
- The shape ronin's growing floor glow of T-072 is gone (it showed before full); his ready cue is the spinning ring,
  as in greybox. The dash keeps its T-072 looks.

## Test seam

`Jam.Fx.TheAuraShowsOnlyWhenFull` replaces `Jam.Fx.TheAuraGrowsWithTheChargeAndSaysReady` (the author changed the
behaviour; red first: the aura was on at 0, 0.5 and 0.99).
