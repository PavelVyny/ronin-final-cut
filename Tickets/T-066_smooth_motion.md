# T-066 Smooth motion and transitions

**Block:** session 5, polish · **Status:** done

## Goal

The author, Sun 15 Nov 02:40: special attention to smooth movement, animations and the transitions between them.
(Plan step 2.) The jump of the whole model when moving was a separate bug, fixed before this ticket (the hero's
export left the root bone where the previous clip ended).

## Decision

- **Actions crossfade.** An action cut short by a new one (strike A into strike B, a strike into a throw) fades out
  under the new one over the blend time, from the weight it had; before, the pose popped to the new clip.
- **The Publisher turns at 540 degrees per second** toward his facing instead of snapping to it. The hero still
  turns at once to the cursor (the author's Hades reference).
- **Backpedalling.** The hero faces the cursor and may walk away from it: when he moves well behind his facing
  (the cosine below -0.35) the run plays backwards (`JamPoseRules::StrideSign`). Sideways stays forward.
- **A dash leaves the afterimages** of T-064 (without the lean): the dash reads as the same neon speed.
- **The Publisher's walk** plays at up to five times its own speed (was 3.5): a fast heavy stomp, still a little
  slide at full speed.

## Test seam

`Jam.Pose.MovingAgainstTheFacingBackpedals` (red first against a stub). The crossfade, the turn and the dash trail
are presentation, checked from the game camera.
