# T-001 Skeleton: folders, game mode, arena map, balance data, fixed-step sim driver

**Block:** A · **Status:** done

## Goal

The game starts on an arena map with our game mode, and an empty fight simulation ticks at a fixed step.

## Scope

- Folders `Core`, `Sim`, `Game`, `View`, `UI`, `Tests` under `Public/` and `Private/`.
- `FJamBalanceData` (every number of the fight) read by `JamConfig::Load` from the flat section `[JamBalance]` of
  `DefaultGame.ini`, one line per field; a missing line is reported by name.
- `FJamFight` state and `JamFightRules::Step(Fight, Intents, Balance)`: empty shell that only advances the tick.
- `UJamFightSubsystem` (world subsystem): accumulates frame time, runs whole fixed steps, exposes the state.
- `AJamGameMode`, map `/Game/Maps/Arena` (a flat round floor of basic shapes, light, sky), set as the default map.

## Test seam

- `JamStepRules::StepsFor(Accumulated, FrameDt, StepDt, MaxSteps)`: how many fixed steps a frame runs and what
  remains. `Jam.Sim.FixedStep.*`: a short frame runs none and keeps the remainder, a long frame is capped.
- `JamBalanceRules::Problems(Balance)`: a list of broken numbers (zero health, thresholds out of order).
  `Jam.Balance.DefaultsAreValid`, `Jam.Balance.ReportsBrokenThresholds`.

## Done

The packaged-style launch (`-game`) opens the arena; tests green; `check_code.py` layering lists know `Game`.
