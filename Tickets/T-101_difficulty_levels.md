# T-101: difficulty levels

Status: done (Sun 15 Nov 2026, session-6a)

## Asked

A DIFFICULTY row right under NEW GAME on the main menu (SETTINGS a row down): `DIFFICULTY  < NORMAL >`, cycled with
the side arrows and with clickable arrows, EASY, NORMAL, HARD. NORMAL by default and exactly the build before it. The
choice is saved between launches and kept by a retry and PLAY AGAIN. No new mechanic: the levels scale numbers that
already exist, one block per level in `[JamBalance]`, tuned with the bot.

## Done

- Rules: `JamDifficultyRules` (`FromIndex`, `ScalesOf`, `Apply`). A level scales the hero's max health, the cash
  stack's heal, and two new fight-wide scales, `HeroDamageScale` (every hit on the Publisher: katana, shurikens,
  Ultimate, on top of the first cut's 1.4) and `BossDamageScale` (every hit on the hero). Both are 1 in the ini and
  copied into the fight at its start; `JamFightOps::Hurt` applies them, the one place damage is dealt.
- `[JamBalance]` blocks, unchanged from the author's starting values:
  EASY health 1.5, damage 1.3, heal 1.5, Publisher 0.8; NORMAL all 1; HARD 0.8, 0.9, 0.7, 1.25.
- The fight host keeps the level (`UJamFightSubsystem::SetDifficulty`); every `Restart` applies it and the checkpoint
  is taken from the scaled balance, so a retry and PLAY AGAIN play the same level. NEW GAME sets it and restarts the
  fight that was set up behind the menu.
- Menu: `EJamMenuItem::Difficulty` (a selector: `JamMenuRules::IsSelector`); left and right step it round, Enter steps
  it on, and a click on the left half of its box steps back, on the right half on. The HUD draws a framed box with an
  arrow at each end and the level between. A label too long for the room left of its control is drawn smaller. Saved
  as `Difficulty` with the volumes in GameUserSettings.ini.
- Bot, 60 fights per level (20 seeds, three ways of answering the cards):

  | Level | Wins | Median fight | Closest win | Median lowest health |
  |---|---|---|---|---|
  | EASY (180 health) | 60 of 60 | 52 s | 148 | 96% |
  | NORMAL (120) | 60 of 60 | 69 s | 72 | 93% |
  | HARD (96) | 60 of 60 | 76 s | 4 | 90% |

  The starting values already give what was asked: EASY never comes close, NORMAL is the build before, and on HARD the
  bot's closest win is 4 health of 96: a close fight even for a bot that is quicker than a person. No tuning was
  needed, so the blocks stay at the starting values.
- Tests, red first: `Jam.Difficulty.NormalLeavesEveryNumberUnchanged`, `Jam.Difficulty.EasyAndHardApplyTheirScales`,
  `Jam.Difficulty.TheChoiceSurvivesARetry`, `Jam.Menu.TheDifficultyRowCyclesItsLevels`,
  `Jam.Bot.EveryDifficultyIsWinnable`. `Jam.Menu.TheMainPageStartsSetsUpOrExits` and
  `Jam.Menu.SettingsOpenAndApplyOnTheWayBack` now expect the main page the author asked for (SETTINGS a row down).

## Not checked

- How the row looks: no game was run. The label and the level shrink only if they would not fit their room.

## After session-6a

- The author: it plays well; the row should fit the menu and the bands be less see-through. The DIFFICULTY label now
  ends at the row's middle and the unframed arrows and level follow it, one centred line like the other rows; the
  main page's bands went from 0.35 and 0.7 (selected) opacity to 0.62 and 0.85. Test: the layout check in
  `Jam.Menu.TheDifficultyRowCyclesItsLevels`, red first. Built as session-6b.
