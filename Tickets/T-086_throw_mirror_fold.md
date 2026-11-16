# T-086 The throw no longer folds the torso: the mirror keeps each bone's roll

**Block:** session 5, Sunday evening fixes before session-5q · **Status:** done

## Goal

The author on session-5p: the shuriken throw still crumples the hero's upper body, part of the torso folds in on
itself, and not only when backpedalling (T-084 did not fix it). Find the real cause from the data: the mirrored
quick-cast clip (T-065), the layered blend from the spine up, or the crossfade weights.

## Cause, from the bones

Logged in headless Blender through the throw (scripts in the session scratchpad, numbers in the DEVLOG):

- The Meshy rig's centre bones have their X axis up and Z sideways, not X sideways. `hero_export.py` mirrored the cast
  as `flip @ partner @ flip` in armature space, which is a true mirror only for bones whose X axis is the mirror's.
  Every bone's rest came out 174 to 184 degrees off its own mirror image.
- In Throw_L that put the pelvis 178 degrees off its rest (59 in the source cast) and the left hand 171 (10 in the
  source). Old against fixed Throw_L in world space: every bone about 180 degrees apart about its own length, the
  joints within 1 to 4 cm. The skeleton kept its shape, each bone was turned half round, so the skin twisted; in a
  crossfade the spine lerped between 0 and 180 against an untwisted pelvis, and the torso folded.
- Not the cause: the layered blend and the crossfade weights; they blend what the clip gives.

## Decision

The mirror takes the partner's change from rest, reflects it and puts it on the bone's own rest:
`flip @ partner @ partner_rest^-1 @ flip @ own_rest`. Against the true mirror the error is 0.0 degrees on every bone,
and each bone's turn off rest equals its source partner's. Throw_L was exported again and imported over itself. The
T-084 warp fade stays: it does no harm.

## Test seam

Offline data (Blender logs of the exported FBX). The clip is an asset; no `Jam.` test can see a skin twist.
