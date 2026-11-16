# T-020 Ultimate

**Block:** D · **Status:** done (hero side; the boss only takes a stolen Ultimate, cut line item 3)

## Goal

The fifth mechanic: a meter filled by the damage the hero deals; when it is full, E winds up in slow motion and
slashes along a long line in front of him.

## Scope

- `EJamMechanic::Ultimate`, `MechanicCount = 5`, `EJamButton::Ultimate`, `FJamIntent::bUltimate` (E),
  `FJamUltimateState` (meter, pending slash), events `UltimateStarted` and `UltimateSlash`.
- Pure `JamUltimateRules`: `OnLine` (the geometry of the slash), `Charge` (the step's hit events fill the meter of the
  side that dealt them), `Press`, `Land`. The meters fill before the slashes land, so a slash never pays for the next.
- Balance in `[JamBalance]`: `UltimateGainPerDamage=0.0015`, `UltimateWindUp=0.35`, `UltimateRange=1600`,
  `UltimateHalfWidth=90`, `UltimateDamage=150`.
- Presentation: the world runs at 0.3 while a slash winds up (read from the state every frame, so a restart cannot
  leave it slow), a magenta ring during the wind-up, the slash line, `E ULTIMATE 64%` / `READY` in the HUD.
- The bot slashes when the meter is full and the boss is within reach.
- `Jam.Bot.EveryStealOrderIsWinnable` plays seeds until each of the 20 orders has two runs (cap 600 seeds): the
  seeds of the other bot tests no longer reach every order with five mechanics.

## Test seam

`Jam.Ultimate.LineTouchesWhatIsInIt`, `Jam.Ultimate.MeterFillsByDamageDealt`, `Jam.Ultimate.SlashHitsAlongTheLine`,
`Jam.Ultimate.MissesOffTheLine`, `Jam.Ultimate.NeedsAFullMeterAndTheMechanic` (red first with a stub rule, 4 of 5
failing), `Jam.Bot.SlashesWhenTheBossIsInReach` (written together with the bot code, not seen red).

## Done

127 `Jam.` tests green, 0 code check warnings. Bot: 60 of 60, median 121 s (the floor of the range is 120: the first
tuning, damage 300 and gain 0.0025, gave 117 s). Clip `Clips/t020_ultimate.mp4`.
