# T-098: a window never larger than the desktop

Status: done (Sun 15 Nov 2026, session-5w)

## Seen

The author's run of session-5v showed only the top-left part of the game: the menu's centre at about 70% of the
screen's width, the rest off the screen. The saved settings
(`%LOCALAPPDATA%/JamGame/Saved/Config/Windows/GameUserSettings.ini`) had `FullscreenMode=2` (windowed) at
3840x2160, a window larger than the desktop it was on.

## Done

- `JamMenuRules::FitWindow`: a window keeps its size if it fits the desktop, else it is scaled down, its shape kept,
  to 90% of the desktop each way. Test: `Jam.Menu.AWindowFitsTheDesktop`, red first.
- `JamSettings::ApplyGraphics`: a windowed resolution picked in the menu goes through FitWindow.
- `JamSettings::FitSavedWindow`, called at the start of every run except a bot's: a saved window that does not fit is
  fitted and saved, so the author's current settings mend themselves on the next launch.

## Not checked

Not run in a game window (the author validates in play). The monitor's real size and Windows' display scaling were
not read here; if the picture is still too large in the borderless mode, Windows' scaling of the game is the next
suspect.
