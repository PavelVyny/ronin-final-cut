# T-084 A throw while backpedalling keeps the upper body

**Block:** session 5, Sunday quick fixes before session-5p · **Status:** done

## Goal

The author: throwing shurikens while running backwards makes the hero's upper body vanish or twist away. Max 30
minutes; if not clean, the simplest safe fix.

## Decision

The simplest safe fix: the T-078 warp (the hip twist and the spine's turn back to the aim) fades out as an
upper-body action fades in (`JamPoseRules::WarpWeight`), and a full-body action takes it all. Under a throw the legs
run their clip straight and the throw owns the chest, as before T-078. Not checked by eye in a backpedalling throw
(the bot rarely does it); the rule is tested.

## Test seam

`Jam.Pose.AThrowTakesTheChestFromTheWarp` (red first).
