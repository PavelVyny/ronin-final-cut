# T-102: a sharp first launch, a Development build that shows what Shipping shows

Status: done (Mon 16 Nov 2026 UTC, session-6c)

## Asked

1. The jam asks for a Development build named `TeamName_GameName.zip`. Package the final as Development (Shipping as a
   fallback) and check it first: it starts from the zip, no debug text or stat overlay anywhere, no test, demo or bot
   mode runs on its own, the console key does not break input, the frame rate is close to Shipping. The download
   should read `Pavyny_RONIN_FINAL_CUT.zip`.
2. The first launch is blurry in Borderless and sharper in Fullscreen on the author's 3440x1440 screen. A first launch
   is borderless at the desktop's size with the scene at 100% (or the DLSS mode on an RTX card), and no saved setting
   leaves a borderless window rendering smaller.

## Cause of the blur

- The game was not DPI aware (`bAllowHighDPIInGameMode` off, the engine default), so Windows told it a 3440x1440 screen
  at 125% was 2752x1152 and stretched its picture over the screen.
- A fresh install leaves `sg.ResolutionQuality` at 0, so `r.ScreenPercentage` stayed 0 and TSR rendered at the engine's
  display-based default (`r.ScreenPercentage.Default.Desktop.Mode=1`): about 72% of 1152 lines, some 828.

## Done

- Rules: `JamDisplayRules::Settle` (tests `Jam.Display.*`, three, red first): a first launch is borderless at the
  desktop's size; a borderless window always covers the desktop; a window fits it (T-098); fullscreen keeps its picked
  size; the scene's share is always 100. A third test checks the DPI setting in the engine ini.
- `bAllowHighDPIInGameMode=True` in `DefaultEngine.ini`. The HUD scales with the screen's height, so it keeps its size.
- `JamSettings::SettleDisplay` replaces `FitSavedWindow`: it settles the saved display at the start of a run (a run
  whose window the command line sets keeps it), marks it with `DisplaySettled=1`, and logs the numbers before and
  after. The menu's graphics settings go through the same rule. DLSS sets its percentage after it on RTX cards; the
  DLSS session's 94c554d makes that percentage take (the user settings had set it at a higher priority).
- Development build: the F5/F6 debug keys and their hint are editor-only; a packaged build turns off engine screen
  messages and clears the console keys. `-JamFrameLog=<file>` writes the average frame rate every 5 s.

## Checked

- First launch with the author's `%LOCALAPPDATA%` `GameUserSettings.ini` renamed (and put back after), log lines:
  - before (Development of 830797f): not DPI aware, borderless 2752x1152, `r.ScreenPercentage` 0, Desktop.Mode 1,
    `sg.ResolutionQuality` 0, TSR: about 72%;
  - after (Session6d, Development of 94c554d): per-monitor DPI aware, `first launch, mode 1 2752x1152 scale 0 -> mode 1
    3440x1440 scale 100 on a 3440x1440 desktop`, DLSS QUALITY with `r.ScreenPercentage` 66.7 on the RTX 4070 SUPER;
    with `-NoDlss` 100.
- Frame rate in a 50 s bot fight (`-JamDemo`, 3440x1440): Development 155 fps with DLSS and 118 fps without; Shipping
  of 32f6db1 155 and 120.
- Development: no automation, demo or bot run without its flag; 420 captured frames of a fight show no debug text. The
  menu, comic and end screen were not captured: the only non-Shipping draw path is now editor-only and screen messages
  are off. The console key is cleared in code, not tested by pressing it.
- 318 tests, 0 warnings at 32f6db1.

## Not done

- itch.io names a butler upload `<slug>-<channel>.zip`; the display name `Pavyny_RONIN_FINAL_CUT.zip` is set by the
  author in Edit game, where the older upload is hidden.
- The Development zip carries `JamGame.pdb` (about 376 MB unpacked).
