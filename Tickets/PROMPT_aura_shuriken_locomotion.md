The author played session-5n: "everything else is great". Two small fixes, then the next block: directional locomotion.

## Part 1: two fixes (test first, red then green)

1. The Ultimate charge aura says nothing while the meter fills. Bring back the Ultimate meter as its own bar under the hero's health, in every tier, as it was before T-072. The red aura under the hero shows only when the meter is full (and through the wind-up), never earlier. Shape ronin and greybox keep what they have now for the ready state, adjusted the same way: the bar always, the ready cue only when full.

2. The hero's shurikens: 30% more shurikens per second, the same damage per second. So the hero's cooldown goes from 0.4 to about 0.31 s and the damage per shuriken from 9 to about 6.9 (round as the code allows, keep DPS within 2%). This is the hero only: the Publisher's stolen shuriken must keep exactly the damage, spacing and speed he has now. `ShurikenCooldown` and `ShurikenDamage` are shared with his stolen copy (scaled by `BossStolenCooldown`, `BossShurikenCooldownScale`, `BossStolenDamage`), so either give him his own base keys or compensate his scales, and pin his current numbers with a test before the change. The hero's explosive shuriken (`ShurikenExplosiveEvery=3`) would then explode 30% more often: ask the author with an interactive question whether to keep explosions per second the same (every 4th) or leave every 3rd.

Then tests, Shipping package, push to itch, DEVLOG. One line to the author when it is live.

## Part 2: next block, directional locomotion (start after Part 1 is live)

The author: wherever the hero moves, his legs always run straight forward along his facing; the feet should step in the direction he is actually moving (strafing, backpedalling, diagonals). In another project they used the Orientation Warping node (Animation Warping plugin) in an Animation Blueprint: a forward walk sequence, Local To Component, Orientation Warping fed by the locomotion direction angle, Component To Local, plus a lean blend space driven by yaw delta. That worked for them; it is only an idea, not a requirement.

Our hero has no Animation Blueprint: `UJamPoseInstance` blends idle, walk and one action in C++. Before writing code, investigate and come back with a short comparison and a recommendation, asked as interactive questions:
- Orientation Warping (or the same idea in C++: twist the pelvis and legs toward the move direction while the spine and head keep facing the aim), its cost in our C++ pose instance versus moving to an Animation Blueprint;
- directional clips (strafe left and right, backpedal) from the Meshy animation library, their credit cost, blended by a direction angle, like a 2D blend space;
- a lean into turns and stride warping (play rate tied to ground speed), which also fixes the Publisher's sliding walk (he moves six times faster than his clip).
Keep the decision of which direction to move in the pure rules with tests (`JamPoseRules`), the view only applies it. Show the result in a clip with the hero circling the Publisher and backpedalling.
