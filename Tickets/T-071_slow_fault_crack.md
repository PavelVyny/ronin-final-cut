# T-071 The crack plays slowly along a jagged fault line

**Block:** session 5, Sunday feedback · **Status:** done

## Goal

The author played session-5m, Sun 15 Nov: the degradation crack (T-068) is too fast, the screen splits and slides
apart before you can see what happened. Make it 2 to 3 times longer and tune it by eye, so it is clear which half went
where. The split line must not be straight: a jagged fault line with a random-looking path across the screen, glowing
neon along the edges. The fight keeps holding while it plays.

## Decision

- **Three beats, 2.1 s in all (was 0.7 s):** the crack runs down the frozen frame from top to bottom with a white-hot
  head (0.5 s), glows whole and flickers (0.3 s), then the two halves slide apart and off the screen, slow at first
  (smoothstep, 1.3 s). The fight holds for all of it, as before (`JamCrackRules::Seconds`).
- **The fault line** is cut by midpoint displacement: the first split always bends 7 to 13% of the width off the
  straight line, every further split pushes less (roughness 0.6, six levels), so it bends at every scale like a real
  crack, and it stays 20% clear of the screen's edges. It keeps its shape for a seed; every crack has another.
  Six short side branches split off it as the head passes and leave with their half.
- **Which half went where:** the left half takes a cyan tint and edge, the right a magenta one; both dim as they go,
  so the new look behind them reads as the one in front. A white flash fills the gap as they come loose.
- Checked by eye on a demo capture at the first cut: the old 3D hero leaves with the left half while the shape ronin
  stands in the gap. The drawing moved to `JamHUDCrack.cpp` (JamHUD.cpp was growing).

## Test seam

`Jam.Crack.RunsDownThenHoldsThenParts`, `Jam.Crack.HalvesPartSlowlyThenLeave`,
`Jam.Crack.TheFaultWandersAndKeepsItsShape` (replace the two T-068 tests on the old opening; red first against
stubs). The first shape (alternating knots) passed the tests but read as an even zigzag on screen; replaced after
the look check.
