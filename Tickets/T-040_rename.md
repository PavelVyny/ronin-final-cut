# T-040 Rename to RONIN: FINAL CUT

**Block:** session 5 · **Status:** done

## Goal

QUESTIONS.md Q1: NEON RONIN is a Steam game (app 1929540, TYPE-III); the author picked RONIN: FINAL CUT (Sat 14 Nov
11:50). Every name a player sees changes; the itch page title is the author's.

## Scope

- `JamTexts::WinTitle`: "RONIN: FINAL CUT" (was "NEON RONIN: DIRECTOR'S CUT").
- The window title: `ProjectDisplayedTitle` in `[/Script/EngineSettings.GeneralProjectSettings]` (it showed JamGame).
- Unchanged on purpose: the module and project name `JamGame` (code, not seen), the hero's "RONIN" health label.
  The zip name is `Pavyny_RoninFinalCut` from the next build on.

## Test seam

`Jam.Hud.TheGameIsCalledRoninFinalCut` (red first on the old title and the missing window title).

## Done

185 `Jam.` tests green, code check without warnings. Clip `Clips/t040_final_cut_title.mp4` (130 s, bot): this run the
bot died at the very end and retried, so the clip shows the loss title and the FINAL NOTE card with the Katana; the
win title is checked by the test only.
