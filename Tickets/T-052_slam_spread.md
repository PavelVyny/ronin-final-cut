# T-052 The five-circle slam spreads out

**Block:** session 5 · **Status:** done

## Goal

The author's playtest of `session-5c` (Sat 14 Nov 16:30): the three-circle pattern is fine, but the five circles land
together and paint one solid patch. Wanted: two circles right next to the Publisher, the other three spread wider,
with a real gap between them, so there are pockets to stand in between the circles, not only outside the group.
Keep the reachable safe spot and add a test for the minimum gap.

## Scope

- `JamSlamRules::Pattern` with five circles: two at the Publisher's flanks (side radius, centred a half gap past it),
  the one on the hero (slam radius, placed first, always kept), two wide behind the hero at 55 degrees either side of
  his way back, at slam radius + side radius + gap. Landing order from him outwards: flanks, hero, wide.
- Each circle takes the nearest place with room on a ring around its anchor: the wanted direction, then turning
  15 degrees at a time either way, then a ring 100 cm wider (8 rings). Room means on the floor, off the cash and at
  least `SlamGap` (200 cm) from every circle placed before it. A circle with no room is left out.
- The three-circle pattern and the single circle are unchanged; the stream draws the same three numbers for any count.
- Numbers in `[JamBalance]`: `SlamGap=200`, `SlamSpreadAngle=55`.

## Test seam

`Jam.Slam.FiveSpreadAroundHimAndTheHero` (the shape), `Jam.Slam.FiveCirclesKeepAGapBetweenThem` (the sweep: 4936
five-circle patterns over the whole arena, the narrowest gap 200 cm). Red first on the old rule. The T-050 sweep
still finds a spot to walk to in every one of the 14808 patterns, and all 4936 fives keep their five circles
(4 rings kept only 2350: near the wall the floor ran out, so the search goes up to 8 rings).

## Bot table (20 seeds per policy)

| Policy | T-051 | T-052 |
|---|---|---|
| left | median 78 s | median 73 s |
| right | median 67 s | median 73 s |
| mixed | median 81 s | median 91 s |
| all | 60 of 60, median 78 s, closest win at 32 | 60 of 60, median 79 s (57 to 175), closest win at 32 |

## Done

177 `Jam.` tests green, code check without warnings. Clip `Clips/t052_slam_spread.mp4` (75 s, bot): one circle,
then three, then from the second cut five separate circles with floor between them.
