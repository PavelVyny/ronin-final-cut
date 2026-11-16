# T-019 Parry

**Block:** D · **Status:** done (hero side; the boss only takes a stolen parry, see below)

## Goal

The fourth mechanic: Q opens a short window; a projectile that reaches the fighter inside it turns round and flies
back at the one who threw it, faster and three times as hard.

## Scope

- `EJamMechanic::Parry`, `MechanicCount = 4`, `EJamButton::Parry` and `EJamAction::Parry`, `FJamIntent::bParry` (Q),
  `FJamParryState` on the fighter, events `ParryStarted` and `Deflected`.
- Pure `JamParryRules`: `CanStart`, `Start`, `IsActive`, `Deflect`. The hook is `AdvanceProjectiles`: a hit on a
  parrying target becomes a deflection, an explosive shuriken included (it explodes on the thrower).
- Balance in `[JamBalance]`: `ParryWindow=0.25`, `ParryCooldown=0.8`, `ParryReturnSpeed=1800`, `ParryDamageScale=3`,
  `ParryReturnLife=1.5`.
- HUD row `Q PARRY`, a cyan ring while the window is open, a white flash on a deflection.
- The bot parries a lone projectile and dashes through a fan (one window does not cover a fan: with parry first
  everywhere it won 16 of 60).
- The boss: a stolen parry is only taken. The Publisher never presses it (cut line item 3, waiting for the author).
- Tests that assumed three mechanics count from `JamSim::MechanicCount` now: the bot steal orders (12), the last
  card (takes two), the retry and steal counts. The take-all card is answered with 0 by the bot whatever its policy.

## Test seam

`Jam.Parry.WindowOpensAndCloses`, `Jam.Parry.DeflectsAMemoBackAtTheBoss`, `Jam.Parry.TheReturnHurtsTheThrower`,
`Jam.Parry.WithoutTheWindowTheMemoHits`, `Jam.Parry.NeedsTheMechanic`, `Jam.Bot.ParriesAMemoBeforeDashing`. Red
first (stub rule, then the bot test alone).

## Done

121 `Jam.` tests green, 0 code check warnings. Bot: 60 of 60 in all 12 steal orders, median 134 s, closest win at 36
health. Clip `Clips/t019_parry.mp4`.
