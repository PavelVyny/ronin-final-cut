# T-034 The hero turns to the mouse at once, a neon pointer

**Block:** session 4 (playtest fixes) · **Status:** done

## Goal

The author's playtest of the session-3 build: "Controls feel sluggish. Aiming is hard. I want a visible cursor and
the hero turning to it instantly, with no lag or smoothing, like Hades 2." No crosshair (answer in chat, Sat 11:50).

## What was in the way

- The simulation had no turn smoothing, but the model only turned on a 60 Hz step, toward an aim read through last
  frame's camera, from the body's simulated position rather than where it is drawn. On a screen faster than 60 Hz
  the turn stepped.
- The pointer was the small Windows arrow, lost on the dark arena.

## Scope

- `JamAimRules::FacingTo`: the way from where the hero is drawn to the cursor; the last facing when the cursor is on
  him. `JamAimRules::FollowCamera`: the floor point under a still cursor slides with a camera that only slides.
- Glue: the controller keeps the camera it read the cursor through; after the camera moved, the game mode turns the
  hero model on every rendered frame (`AJamFighterView::Face`). Not in the bot demo, which keeps the simulated facing.
- A hardware pointer (`Content/Slate/Cursors/NeonPointer.png` and `@2x`, drawn by `Tools/Art/make_cursor.py`):
  Windows draws it, so it never trails the mouse. `HardwareCursors` in `DefaultEngine.ini`, the folder staged with
  `DirectoriesToAlwaysStageAsUFS` for the packaged game.
- Out: the katana wind-up stays at 0.1 s (the author took the option without it).

## Test seam

`Jam.Aim.ShownHeroFacesTheCursor`, `Jam.Aim.CursorPointFollowsTheCamera`: red first (2 failing on stubs).

## Done

145 `Jam.` tests green. The demo log shows both pointer images loaded (`LogInit: Loading Cursor ...NeonPointer.png`,
`@2x`). No clip of the turn: the bot demo drives the simulated facing, and a captured frame has no hardware cursor;
the feel is for the author's hands on the next itch build.
