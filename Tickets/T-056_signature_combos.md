# T-056 A signature combo per stolen mechanic

**Block:** session 5, fight · **Status:** done

## Goal

The author's playtest: the fight comes down to parry, dodge the circles, dodge the fan, close in, hit. Each cut must
change how he fights. The author's answer (Sat 14 Nov 18:30): one chain per stolen mechanic, unlocked when he steals
it, so the steal order changes the fight. Readable from the top-down camera, fair: a telegraph on every link and a
gap of at least 0.3 s between links.

## Chains

| Unlocked by | Chain |
|---|---|
| His own, from the first cut | Slam pattern, then a fan aimed into the safe pocket |
| Dash | Dash in, the slam goes down on arrival |
| Shuriken | Fan, then a burst of three shuriken led at the side the hero dodges to |
| Parry | Shield stance (T-038), then an instant fan the moment it drops |
| Ultimate | Walk or dash to line up, then the line slash (T-038) |
| Katana (FINAL NOTE only) | Fan, then a katana rush |

## Scope

- A pure `JamComboRules`: which chains he owns (ledger + cuts), which one he picks (seeded stream, not the same twice
  in a row), the link schedule (what fires on which step). The brain runs the schedule through the same intents.
- The chain ends with the usual `BossRecovery` pause.
- Numbers in `[JamBalance]`: `ComboCooldown`, `ComboLinkGap` and the burst count.

## Test seam

`Jam.Combo.*`: a chain is only offered with its mechanic, links keep the gap, the pocket fan aims at the pocket,
the stream picks, never the same chain twice in a row. Bot table stays 60 of 60.

## As built

- `EJamCombo`, `EJamLink`, `FJamComboLink`; `FJamBossState` holds the running chain, its links, the last chain and
  `ComboReadyTick`. `FJamIntent::bCombo` asks for one.
- `JamComboRules`: `Available` (from the first cut: Pocket with the hero within slam range, DashSlam for a far hero,
  FanBurst for a hero out of blade reach, ShieldFan, FanLine on a full meter, FanRush), `Pick` (seeded, never the last
  one twice), `Plan` (the links by step), `LinkNow`, `IsRushing`, `PocketAim` (the pocket the pattern offered when it
  went down, after `SlamReaction`).
- `JamBossRules`: a ready chain comes right after the shield; `Steer` aims and presses the stolen buttons of the link,
  `RunLink` fires his own slam and fans, and the end sets his pause and the 5 s cooldown. A running chain is never
  "busy". A card clears a running chain.
- The Ultimate chain is "fan, then the line along it" instead of "walk or dash to line up": the line reaches 1600 cm,
  so lining up never mattered; the step aside that dodges the fan also clears the line.
- View: a turning red ring under him during the 0.35 s tell. `DrawStances` split out of `DrawCircles` (61 lines).
- `[JamBalance]`: `ComboCooldown=5`, `ComboTell=0.35`, `ComboLinkGap=0.4`, `ComboBurstCount=3`, `ComboBurstGap=0.2`,
  `ComboRushSeconds=1.2`.

## Test seam (as built)

`Jam.Combo.OnlyWhatHeStole`, `Jam.Combo.LinksKeepTheirGap`, `Jam.Combo.NeverTheSameTwiceInARow`,
`Jam.Combo.DashSlamLandsOnArrival`, `Jam.Combo.PocketFanAimsAtThePocket`, `Jam.Combo.ShieldDropsIntoAFan`,
`Jam.Combo.EndsWithHisPause`. Red first: 7 of 7 on stubs.

## Bot table

60 of 60, median 97 s (70 to 159; T-038: 108 s), closest win at 26 health (T-038: 66). Left 97 s at 26, right
102 s at 46, mixed 89 s at 40.

## Done

200 `Jam.` tests green, code check without warnings. Clip `Clips/t056_combos.mp4` (60 s, the bot gives away Dash
and Shuriken): the five-circle pattern with the hero in the pocket, the chains after each cut.
