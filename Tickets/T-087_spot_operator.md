# T-087 The Publisher's spot trails him like a hand works it

**Block:** session 5, Sunday evening fixes before session-5q · **Status:** done

## Goal

The author: the Publisher's spotlight about 20% wider in diameter; it must not follow him pixel for pixel but trail
him by about 0.5 s on a smooth spring and sway slightly, like an operator trying to keep him lit. The follow maths
in a pure rule.

## Decision

- `JamSpotRules::Follow`: a critically damped spring stepped exactly (stiffness 2 / Lag), so it eases after him,
  never swings past, trails a steady walk by Lag = 0.5 s and stays stable at any frame time.
- `JamSpotRules::Sway`: two slow waves per axis at periods that never line up, at most 25 cm.
- Cones 9.6 and 21.3 degrees (was 8 and 18): the pool a fifth wider across.

## Test seam

`Jam.Spot.*` (four tests, red first).
