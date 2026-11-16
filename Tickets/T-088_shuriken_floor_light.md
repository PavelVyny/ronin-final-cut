# T-088 Flying shurikens light the floor under them

**Block:** session 5, Sunday evening fixes before session-5q · **Status:** done

## Goal

The author: flying shurikens light the floor under them in the shuriken's colour, the explosive one slightly
brighter; one light per shot with a tight radius, no shadows, and a cap on the number at once.

## Decision

In the full art only, a pool of six point lights, no shadows, 240 cm reach, 12 cd, at the drawn shot, in its colour
(the hero's cyan, the Publisher's red, the explosive yellow). `JamShotLightRules::Lit` picks the shurikens, the
newest first, up to the cap; `Brightness` gives an explosive one 1.5 times the light (the author's pick).

## Test seam

`Jam.ShotLight.*` (two tests, red first). The look is not checked by eye.
