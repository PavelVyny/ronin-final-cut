# T-068 The crack between two looks

**Block:** session 5, polish · **Status:** done

## Goal

The author, Sun 15 Nov 02:40: when the stage of the degradation changes it should be visually interesting, like a
vertical crack through the whole screen that moves left and right and changes the graphics as it passes. Grill
answer: the old look is kept as a frozen frame. (Plan step 4.)

## Decision

- **Freeze, then switch.** When the wanted layers differ from the ones on screen, the camera rig renders the frame
  into a texture (`AJamCameraRig::Freeze`, a scene capture with the camera's post process) while the old look is
  still applied; the new look comes on the next frame, and the fight holds until the crack is open (0.7 s).
- **The crack opens from the middle to both edges** (`JamCrackRules::HalfOpen`, an ease-out), its edges jagged in 36
  strips (`EdgeJag`, a hash: the shape holds while it opens, each crack has its own). The old picture's halves slide
  apart with it, torn at the edges; a white burst fills the gap for 0.15 s; the edges glow cyan (left) and magenta
  (right) with a white core. Drawn by the HUD under its bars and texts, so the HUD stays readable.
- **Every change of look cracks**: each cut, the stun giving everything back, the win, a retry or a restart.

## Test seam

`Jam.Crack.OpensFromTheMiddleToTheEdges`, `Jam.Crack.TheEdgeIsJaggedAndKeepsItsShape` (red first against stubs).
Checked in game at the first two cuts (`-JamDemoSkipEvery=8`).
