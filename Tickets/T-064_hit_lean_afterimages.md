# T-064 A hit is an upper-body lean with neon afterimages

**Block:** session 5, models · **Status:** done

## Goal

The author, Sun 15 Nov 00:30 (from the earlier feedback): the hit reaction is an additive upper-body layer with the
root locked; the legs and the actions are never interrupted. A quick hit-lean pose plus 3 to 4 neon afterimages
fading over about 0.3 s (the Sandevistan look of Cyberpunk: Edgerunners as a visual reference only). No roll.

## Decision

- **A hit starts no action any more.** `JamPoseRules::Shakes` says who is shaken (any hit on the hero, hits of 25
  and more on the Publisher); `CueFor` no longer offers the hit clip. Only the stun still plays the Publisher's
  whole-body stagger.
- **The lean is additive and rotation only.** The pose instance turns every bone from `spine_01` up by the
  difference between `Hit_Reaction` at its peak frame and its first frame, `LeanWeight` of the way: a snap in over
  0.05 s, an ease back until 0.35 s. Root, pelvis and legs never move for it, so the walk and a strike under it go on.
- **Afterimages (the hero only):** four frozen copies of his pose (`UPoseableMeshComponent`), left where he was at
  the hit and every 0.05 s after, cyan and magenta in turn, each fading out over 0.3 s (`M_JamGhost`: unlit,
  additive, brightest at the rim). They belong to the effects layer: gone with the effects, the animation or the
  model.
- The characters' flash now blends the base colour at most 35% of the way to white: a full blend blanked the
  cream-suited Publisher.

## Test seam

`Jam.Pose.HitsLeanWithoutCuttingAnAction` (replaces the hit test of T-061: the behaviour changed on the author's
word), `Jam.Pose.TheHitLeanSnapsInAndEases`, `Jam.Pose.AfterimagesTrailTheHitAndFade`: red first against stubs.
The look is checked in the game (frames after the hero's hit at 3.97 s of the demo run, `-LogCmds="LogJamGame
Verbose"` logs every shaking hit).
