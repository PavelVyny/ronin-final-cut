# T-078 The hero's legs run where he moves, not where he faces

**Block:** session 5, Sunday feedback · **Status:** done

## Goal

The author, Sun 15 Nov afternoon: "Wherever the hero moves, his legs run straight forward along his facing; the feet
should step where he actually moves (strafing, backpedalling, diagonals)." The hero faces the cursor at once and moves
with WASD on his own, so the move and the facing differ all the time. The author chose: orientation warping in C++ in
our pose instance plus directional clips from the Meshy library, blended by the move angle.

## Decision

- **Clips.** The Meshy library has no plain strafe or backwards run; the nearest are four diagonal runs, bought on the
  hero's rig (task `hero_dirs`, action ids 5 `BackLeft_run`, 6 `BackRight_Run`, 9 `ForwardLeft_Run_Fight`, 10
  `ForwardRight_Run_Fight`, 12 credits). `Tools/Art/hero_export.py` exports them as `Run_ForwardLeft`,
  `Run_ForwardRight`, `Run_BackLeft`, `Run_BackRight` (in place already); `Tools/Editor/import_models.py` imports
  them (`JAM_IMPORT_CLIPS` imports only the named clips onto the mesh in the project).
- **`JamPoseRules::MoveAngle`**: the move's angle off the facing, positive to the right.
- **`JamPoseRules::StrideFor(Angle, bDiagonals, MaxTwist, BlendDegrees)`**: each run covers a sector of angles: the
  run straight ahead to 22.5 degrees, the forward diagonals to 105 (so a strafe, the commonest case while circling,
  sits in the middle of one sector and not on an edge), the back diagonals to 157.5, the run played backwards beyond.
  The hips and legs twist the rest of the way toward the move, at most 60 degrees. At a sector edge the two runs cross
  over 16 degrees, so neither the weights nor the twist jump. Without the diagonals (the Publisher) the run turns
  forwards up to 110 degrees and plays backwards beyond.
- **The pose instance** blends the runs on one shared cycle, turns the pelvis by the twist in component space and
  turns the spine back over `spine_01..03` by the twist plus however far the diagonal run turned the chest off the
  plain run's, so the chest and the head keep facing the aim (under an upper-body action only the hips' part).
- **Stride warping by play rate:** `JamPoseRules::StrideRate` plays each run as fast as the ground speed needs, at
  most 3.6 times (the back diagonals at his 600 cm/s); the feet speeds of the runs were measured from the planted foot
  in Blender and checked in the stride gallery. The forward run's speed is now 420 cm/s (was 450: a small slide).
- **Lean into turns:** `JamPoseRules::TurnLean` leans the upper body up to 7 degrees into the facing's turn rate,
  eased, only while he runs.
- The view eases the weights and the twist (6 and 360 per second) so a flick of the cursor never pops the legs.
- **Demo switches** for the clips: `-JamStrideGallery` (the hero faces east and runs eight ways in turn under a front
  camera, `-JamStrideTop` from above; then he circles the standing Publisher facing him, backs off and runs back in;
  then the Publisher walks), `-JamDemoClose` (a closer camera on the bot's fight).
- `JamFighterView.cpp` was over 600 lines: the body's motion (Sync, Face, the runs) moved to `JamFighterMotion.cpp`.

## Test seam

`Jam.Pose.MoveAngleIsToTheRightOfTheFacing`, `Jam.Pose.StrafingRunsTheDiagonalAndTwistsTheHips`,
`Jam.Pose.StrideNeverJumpsAndTwistsAtMostTheLimit`, `Jam.Pose.TheRunKeepsUpWithTheGround`,
`Jam.Pose.TheBodyLeansIntoTheTurn` (red first against stubs). The blend, the twist and the lean are presentation,
checked in the stride gallery.
