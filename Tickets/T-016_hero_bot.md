# T-016 Hero bot and full-fight tests

**Block:** K1 · **Status:** done

## Goal

A bot plays the hero through the same intents, so a whole fight runs in a test in milliseconds. It answers the main
risk of the plan: can a bean beat the boss who holds everything?

## Scope

- `JamHeroBotRules::Decide(Fight, Balance, Stream)` returns an `FJamIntent`: keep a distance, strafe out of the slam
  circle and off the memo lines, use what the ledger still gives, pick a steal option by a policy (first, second,
  random).
- A headless runner: `RunFight(Seed, Policy, MaxSeconds)` returns the outcome, the duration and the hero's lowest
  health.
- A console command `Jam.Bot 1` lets the bot play in the real game (for clips and a smoke run of the build).

## State

- Done in block A: `JamBotRules::Kite` (circle the opponent at a range and shoot), tests `Jam.Bot.AimsAtTheOpponent`,
  `KeepsItsRange`, `WearsDownAStandingBoss`. The bot plays in the real game with the `-JamDemo` command line switch
  (instead of the console command), `Tools/Build/demo_clip.sh` records it.
- Done in block B: `JamBotRules::Play` (uses whatever the hero still owns, dashes out of warning circles and away
  from projectiles about to hit, closes in while the boss is busy), tests `Jam.Bot.LeavesTheSlamCircle`,
  `DivesInWhileTheBossRecovers`, `DashesThroughAMemo`, `HoldsItsOwnAgainstThePublisher`. The demo run plays with it.
- Measured in block B: the bot beats the Publisher in about 12 seconds, the target is 3 to 4 minutes. The balance
  pass belongs to block C, with the fight length as a test.
- Left: steal policies, the headless runner and the tests below.

## Test seam

- `Jam.Bot.FightAlwaysEnds`: 20 seeds, every fight ends as `Won` or `HeroDead` within the time limit.
- `Jam.Bot.EveryStealOrderIsWinnable`: for each order of giving mechanics away at least one seed in N is a win.
- `Jam.Bot.FightLengthIsInRange`: the median duration is between 2 and 5 minutes (target 3 to 4).
- `Jam.Bot.SameSeedSameFight`: the simulation is deterministic.

## Done

Tests green; the numbers in the balance section are tuned until they are. The table of results goes to `DEVLOG.md`.

## As built

- `JamBotRules::Choose(Fight, Policy)` answers the card (`EJamStealPolicy`: Left, Right, Mixed; no random policy, the
  card's own shuffle already makes "left" a random mechanic). `JamBotRules::RunFight(Balance, Seed, Policy, MaxSeconds)`
  plays one life from the first step to a title and returns `FJamBotRun`: outcome, seconds, the hero's lowest health
  and the mechanics in the order the Publisher got them.
- Two habits the bot needed to survive the stolen kit: it walks out of the reach of a katana in its wind-up and
  dashes only when the blade is about to land; without the dash it does not run in at all (next to the boss a volley
  cannot be walked around).
- Tests: `Jam.Bot.ChoosesByItsPolicy`, `BacksOffFromAWindingKatana`, `KeepsAwayWithoutTheDash` on the round numbers;
  `SameSeedSameFight`, `FightAlwaysEnds`, `EveryStealOrderIsWinnable`, `FightLengthIsInRange` on the numbers of the
  game (`[JamBalance]` in `Config/DefaultGame.ini`): 60 fights, 20 seeds for each policy.
- Balance: `BossHealth` 1000 to 4000, nothing else. Results:

| Boss health | Bot | Wins | Median of a won fight | Closest win |
|---|---|---|---|---|
| 1000 | runs in whenever the boss rests | 60 of 60 | 35 s (31 to 46) | 44 health left |
| 4000 | runs in whenever the boss rests | 34 of 60 | 168 s (126 to 175) | 74 health left |
| 4000 | runs in only with the dash | 60 of 60 | 168 s (126 to 182) | 74 health left |

  In the second row every lost fight is one where the Dash went before the last card (0 of 26); every other order
  was won (34 of 34).
- Open after the pass: a hero who circles at range is almost never hit (74 of 100 health is the worst moment of a
  won fight). The Publisher needs an answer to kiting: a volley that leads its target, or a shrinking arena. The
  bean stage is the longest one (1000 health at about 14 damage a second). Both wait for the author's hands on the
  build.
