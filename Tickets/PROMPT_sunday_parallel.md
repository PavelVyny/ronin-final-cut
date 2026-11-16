# Sunday afternoon: one combined task, run in parallel

The author played session-5n: "everything else is great". This prompt replaces `PROMPT_full_tier_look.md` and `PROMPT_aura_shuriken_locomotion.md` (do not run them separately).

## How to run it

You are the orchestrator. Split the work into tracks and hand each to a lead agent in its own git worktree, as many leads as you judge right (the tracks below are a suggestion, merge or split them). Each lead works test first in its worktree, then reports to you: what changed, tests, a screenshot or short clip of the look, open questions. You review every report against this prompt, send it back if it falls short, and land approved branches on main one at a time, running the full test suite after each landing.

The author approved the parallel run in advance: post a one-line forecast (number of agents, rough tokens, time) and start without waiting.

Constraints for the leads:
- The Unreal editor and the Unreal MCP are shared, and a build closes the editor. Agree slots: only one lead at a time works in the editor or builds. Binary assets (`.uasset`, Niagara systems, the level) cannot be merged: give each asset to exactly one track.
- Balance numbers live in `[JamBalance]` in `Config/DefaultGame.ini`; decisions in the pure `*Rules` with `Jam.*` tests, red first.
- Questions to the author go through you, as interactive questions, batched.
- We are past the 12:00 feature stop. Aim to land everything by about 17:00; whatever is not solid by then is left out and goes in the DEVLOG as not done. The final build must be stable: the video is recorded from it at 18:00.

## Track A: two quick fixes (land first, small)

1. The Ultimate charge aura says nothing while the meter fills. Bring back the Ultimate meter as its own bar under the hero's health, in every tier, as before T-072. The red aura under the hero shows only when the meter is full (and through the wind-up), never earlier. Shape ronin and greybox: the bar always, their ready cue only when full.

2. The hero's shurikens: 30% more per second, the same damage per second. Hero cooldown 0.4 s to about 0.31 s, damage per shuriken 9 to about 6.9 (round as the code allows, DPS within 2%). The hero only: the Publisher's stolen shuriken keeps exactly his current damage, spacing and speed. `ShurikenCooldown` and `ShurikenDamage` are shared with his stolen copy (scaled by `BossStolenCooldown`, `BossShurikenCooldownScale`, `BossStolenDamage`), so give him his own base keys or compensate his scales, and pin his current numbers with a test before the change. The hero's explosive shuriken (`ShurikenExplosiveEvery=3`) would explode 30% more often: ask the author whether to keep explosions per second the same (every 4th) or keep every 3rd.

## Track B: the full-art tier must be the most beautiful

The author: the hit effects still look like a lower tier (small flat sparks, red on the hero). The top tier should be as beautiful as we can make it so the fall to the lower tiers is felt. No hardware ray tracing (judges' GPUs may lack it).

1. Remove the flat greybox layer from the full-art tier. `AJamEffectsView::DrawBurst` (`Source/JamGame/Private/View/JamEffectsView.cpp`) still draws plain discs in every tier: hit, slam, explosion, deflect, heal. In the full-art tier Niagara alone shows these events; the discs stay in shape ronin and greybox. Keep anything that carries gameplay information (telegraph circles, the slam warning) unless a Niagara version replaces it clearly. The tier choice is a rule with a test.
2. Rework the T-069 Niagara systems (`/Game/FX/NS_JamHitSparks`, `NS_JamSlamDust`, `NS_JamCashBurst`, `NS_JamTrail`, `NS_JamEmbers`; built on two simple materials, then cut down, the trail never checked by eye): hits get a bright core flash with a short-lived point light, more and longer streaks; slams and blasts a shockwave ring; cash bursts sparkle. Keep the Publisher readable (prefer light and streaks over big opaque sprites, he washed white before). Check every effect by eye from the game camera.
3. Lighting and post process for the full-art tier only: Lumen GI and reflections, bloom, cyan and magenta neon lights in the arena, emissive surfaces that light the floor, slight vignette and grading. Lower tiers turn them off step by step (shape ronin: flat light, no bloom; greybox: unlit and bare). Read the current settings in `Config/DefaultEngine.ini` and the level first and report what is on.
4. Hold 60 fps on a mid-range GPU: log frame times in a busy moment (Ultimate plus slam). If Lumen costs too much, fall back to simpler light rather than dropping the look.

## Track C: directional locomotion

The author: wherever the hero moves, his legs run straight forward along his facing; the feet should step where he actually moves (strafing, backpedalling, diagonals). In another project they used Orientation Warping (Animation Warping plugin) in an Animation Blueprint: a forward walk sequence, Local To Component, Orientation Warping fed by the locomotion direction angle, Component To Local, plus a lean blend space driven by yaw delta. It worked there; it is an idea, not a requirement.

Our hero has no Animation Blueprint: `UJamPoseInstance` blends idle, walk and one action in C++. First a short comparison and a recommendation, sent to the author as interactive questions before code:
- Orientation Warping, or the same in C++ (twist pelvis and legs toward the move direction while spine and head keep facing the aim), in our pose instance versus moving to an Animation Blueprint;
- directional clips (strafe left and right, backpedal) from the Meshy animation library, their credit cost, blended by a direction angle;
- a lean into turns and stride warping (play rate tied to ground speed), which also fixes the Publisher's sliding walk (he moves six times faster than his clip).
The move-direction decision lives in `JamPoseRules` with tests; the view only applies it. Clip: the hero circling the Publisher and backpedalling. This is the riskiest track: if it is not solid by 17:00, leave it out of the build.

## The end

All landed tracks on main, full test suite green, one Shipping package, smoke run, push to itch, DEVLOG and ticket index updated, a clip comparing the same moment in all three tiers. Then one short report to the author: what is live, what was left out, the build name.
