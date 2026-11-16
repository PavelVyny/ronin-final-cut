# T-004 Three cameras on F1, F2, F3

**Block:** A · **Status:** done

## Goal

F1 is a top-down 3/4 camera like Hades, F2 is third person over the shoulder, F3 is first person. The switch is
instant and changes only the camera and the aim source: the simulation does not know which one is active.

## Scope

- `EJamCameraMode` and `AJamCameraRig` in `View`: places the view each frame from the hero view actor and the
  look angles.
- Top-down: fixed yaw and pitch, the cursor is visible, movement is in screen axes.
- Third and first person: the mouse turns the view, the cursor is hidden, movement is relative to the view yaw.
  In first person the hero mesh is hidden for the owner.
- Camera numbers (height, distance, pitch, shoulder offset, FOV) in the balance section.

## Test seam

`JamCameraRules` in `View`:
- `ViewFor(Mode, HeroPos, LookYaw, LookPitch, Settings)` returns location, rotation and FOV.
  `Jam.Camera.TopDownLooksAtTheHero`, `Jam.Camera.ShoulderIsBehindTheHero`, `Jam.Camera.FirstPersonIsAtEyeHeight`.
- `MoveBasisYaw(Mode, LookYaw, TopDownYaw)`: `Jam.Camera.TopDownMovesInScreenAxes`.
- `ClampPitch`: `Jam.Camera.PitchIsClamped`.

## Done

All three cameras play the same fight in PIE; a clip per camera is possible. Tests green.
