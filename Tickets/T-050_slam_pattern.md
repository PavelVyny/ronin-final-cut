# T-050 The slam grows with each cut

**Block:** session 5 · **Status:** done

## Goal

The author's request (Sat 14 Nov 15:45): the Publisher's slam becomes a pattern that grows with each cut. Before the
first cut, one circle on the hero as now. After the first cut, 3 circles: one around him (no standing close), one
exactly on the hero, one behind the hero that cuts off the way back. From the second cut to the end, 5: the same plus
two more beside the hero. Answers in chat: 1-3-5-5-5; the circle on the hero stays 300 cm, the others are smaller; they
land one after another, 0.2 s apart, from him outwards; a landed cash stack is never covered; the dash still protects.

## Scope

- Simulation, `JamSlamRules`: the circle count by the cuts done, the seeded placement, and the nearest spot the hero
  can walk to in time (walking only: the dash may be stolen). A cutter that would shut every way out is left out; a
  circle over a cash stack is pushed off it. Numbers in `[JamBalance]`.
- The boss puts the whole pattern down in one slam and stands until the last circle lands plus his pause.
- The bot walks to the safe spot while circles are on the floor (the old rule left the first circle only).
- View and sound unchanged: each circle fills on its own clock, each landing flashes, thuds and shakes.

## Test seam

`Jam.Slam.TheCountGrowsWithTheCuts`, `Jam.Slam.ThePatternClosesTheBossTheHeroAndTheWayBack`,
`Jam.Slam.EveryPatternLeavesAReachableSafeSpot`, `Jam.Slam.TheCashStaysClear`, `Jam.Boss.SlamGrowsWithTheCuts`,
`Jam.Bot.WalksOutOfEveryPattern`. Red first on stubs (the four `Jam.Slam` tests); the bot test is red on the old
"leave the circle you stand in" rule (at the wall the bot took two circles, 100 to 32).

The safe-spot sweep uses the game's numbers: hero on a 250 cm grid over the whole arena (wall ring included), the
Publisher at three distances and eight angles, 1, 3 and 5 circles, two seeds: 14808 patterns, none without a spot the
hero reaches walking after 0.3 s, checked by an independent step-by-step walk. All 4936 fives kept their five circles.

## Bot table (20 seeds per policy)

| Policy | T-049 | T-050 |
|---|---|---|
| left | median 101 s | median 104 s |
| right | median 96 s | median 83 s |
| mixed | median 84 s | median 80 s |
| all | 60 of 60, median 92 s, closest win at 43 | 60 of 60, median 83 s (63 to 172), closest win at 23 |

The fight got shorter for the bot: he stands until his last circle lands (up to 0.8 s longer), more openings for the
bot, which never misreads a pattern. A person is expected to lose more health to it; not measured.

## Done

174 `Jam.` tests green, code check without warnings. Clip `Clips/t050_slam_pattern.mp4` (50 s, bot): one circle at
the start, three after the first cut (18 s, 28 s), five after the second (44 s).
