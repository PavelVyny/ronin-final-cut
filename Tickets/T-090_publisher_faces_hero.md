# T-090 The Publisher's show-off ends facing the hero

**Block:** session 5, Sunday evening fixes before session-5q · **Status:** done

## Goal

The author: Kung_Fu_Punch ends with him looking to the left. Turn him about 45 degrees counter-clockwise, seen from
the top-down camera, for the whole intro clip, so his final pose faces the hero; if 45 counter-clockwise turns him
away, use the turn that makes him face the hero and say which.

## Decision

Measured in `boss_fixed` in Blender (the clip turns no root, so all of it shows in game): the clip ends turned
clockwise off his idle, head 98, neck 142, chest 156, hips 139 degrees. Counter-clockwise is the right way (his right
is the screen's left from our camera), 45 is too little: the model turns 120 degrees counter-clockwise (yaw -120)
while Kung_Fu_Punch shows, eased out as it fades (`JamPoseRules::ShowOffTurn`). He ends facing the hero, the head 22
degrees to his left, the hips 19 to his right. The cost: the show-off starts turned 120 degrees off the hero.

## Test seam

`Jam.Intro.ThePublisherTurnsWithHisShowOff` (red first). Not checked by eye.
