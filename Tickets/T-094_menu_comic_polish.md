# T-094 Menu, comic and card polish after the author's first look

**Block:** session 5, Sunday evening · **Status:** done, not yet seen in a game window

## Goal

The author's fix list after T-092 and T-093 (`Tickets/PROMPT_menu_comic_polish.md`, kept untracked as the brief):

- The menu's skyline covers ultrawide screens with the tower and its dollar sign in frame; the two fighters keep
  idling instead of freezing after one loop.
- Settings: an opaque panel; the drawn controls are the hit areas; graphics quality, window mode and resolution are
  dropdowns, the three volumes sliders, by keys and mouse.
- Voice lines about 30% louder. A soft ambient track under the comic and the intro that crossfades into the theme when
  the Publisher's intro line ends, or at the skip.
- The comic keeps its own shape with black bars on a wider screen.
- The steal card's choices take a click; the card pops in with a slight overshoot, the choices a beat later, and the
  other phase panels pop in too.
- A light film look in the full-art tier only: grain, rain haze and depth fog, a colour fringe at the edges, a softer
  far distance.

## Decision

- **Skyline:** the engine keeps the vertical field of view, so a wider screen shows more to the sides. The picture is
  outpainted to a 3:1 panorama by fal GPT Image 2.6 (`Art/Concepts/round9_menu_wide_jobs.json`, more sky above the
  tower), and `JamArenaRules::PlaceBackdrop` sizes the plane each menu frame to cover the screen with its top on the
  screen's top, so any overflow goes below the city line. 1% overscan against hairlines.
- **Idle:** the pose extraction clamps a time past the clip's end; `JamPoseRules::LoopTime` wraps it.
- **Settings:** one `JamMenuRules::Layout` gives the rows, the controls, the panel and an open list's options for a
  screen size; the HUD draws from it and the game mode hit-tests it (`Hover`, `Click`, `Drag`, `Release`). Enter or a
  click opens a dropdown, arrows move in it, Enter or a click picks, Esc or a click outside closes it. Left and right
  still step a value; a slider takes a click or a drag along its track.
- **Sound:** `JamVoiceRules::Loudness` multiplies the menu's voice gain by 1.3. The ambient is ElevenLabs Music
  (`Tools/Audio/eleven_ambient.json`); it came back on A, so `Tools/Audio/make_ambient.py` shifts it up three semitones
  to the theme's C, cuts a 32 s loop with a crossfaded seam and levels it to -20 dB. `JamIntroRules::IsThemeUp` says
  which track should be up (the menu and the fight: the theme; the comic and the intro until the Publisher's line has
  ended or the player skipped: the ambient), `JamIntroRules::Crossfade` moves the level over 2.5 s.
- **Comic:** `JamComicRules::Frame` keeps the comic at 16:9 at most, centred; the panels, captions, sound words,
  balloons and the skip hint sit in the frame, a sliding panel is cut at the frame's edge.
- **Card:** `JamCardRules` holds the card's boxes (the HUD draws them, a click finds them) and the pop
  (ease-out-back, 0.3 s, options 0.12 s apart). The stun and ending bands pop the same way, in real time.
- **Film look:** `[JamNeon]` in `DefaultGame.ini` gains the grain, fringe, fog and far-blur numbers;
  `JamNeonRules::FilmLookFor` returns them for the neon light only. The camera carries the grain, the fringe and a
  depth blur focused past the arena with the iris closed; a height fog on the light actor carries the haze. The fog is
  off under the menu, where the skyline stands 100 m out.

## Test seam

Thirteen new or changed tests, red first: `Jam.Pose.TheIdleLoopsForever`, `Jam.Arena.TheSkylineCoversAnyScreen`,
`Jam.Comic.TheComicKeepsItsShapeOnAnyScreen`, `Jam.Intro.TheAmbientGivesWayToTheTheme`,
`Jam.Voice.LinesPlayLouderThanRecorded`, `Jam.Neon.TheFilmLookIsTheFullArtsOnly`, `Jam.Card.*` (two),
`Jam.Menu.TheDrawnSlidersTakeTheClicks`, `Jam.Menu.DropdownsOpenAndPick`, `Jam.Menu.TheLayoutFitsAnyScreen`, and the
changed `Jam.Menu.SettingsStepThroughTheirValues`. Nothing was looked at in a game window, by the author's ask: the
film look's strength, the fog, the crossfade's timing, the dropdowns' look and the skyline's placement are unchecked.
