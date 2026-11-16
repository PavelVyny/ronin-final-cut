# T-092 The main menu: new game, settings, exit, over the two fighters

**Block:** session 5, Sunday evening · **Status:** done, not yet seen in a game window

## Goal

The author: a main menu with basic settings (new game, settings, exit), over a generated background; ideally not a
still picture but the hero idling on one side and the Publisher on the other, looking at each other. Picked: a live 3D
scene behind a generated skyline; settings: graphics quality, window mode and resolution, master, music and voice
volume.

## Decision

- The menu comes before the intro. The fighters stand 2.8 m either side of the centre, face to face, in their idle,
  the camera low on +Y (the hero on the left, the Publisher in his spot on the right); the music plays.
- The skyline `r8_menu_backdrop_b` (fal GPT Image 2.6: the neon city on the hero's side, the Publisher's gold dollar
  tower turning to grey checkerboard on his) stands on the engine plane 90 m past the centre, unlit, out of Lumen,
  only while the menu is up. The plane's UVs were read headless (U along +X, V along +Y) to stand it the right way.
- `JamMenuRules`: the rows of each page, what a key or a click does (arrows and WASD, Enter and Space, Esc and
  Backspace; the mouse hovers and clicks, a click on a value's right half steps it up), values wrap, volumes stop at
  0 and 100%. Graphics apply when the settings page closes, so stepping through resolutions does not flash the screen;
  volumes at once.
- `JamSettings` (glue): UGameUserSettings for the quality, the window mode and the resolution; the volumes in
  GameUserSettings.ini, the master through the audio device, music and voice through their actors.
- A bot run (`-JamDemo`) starts without the menu.

## Test seam

`Jam.Menu.*` (five tests, red first). The look, the framing and the plane's facing are not checked by eye.
