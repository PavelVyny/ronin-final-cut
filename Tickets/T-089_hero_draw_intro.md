# T-089 The hero draws his blade as the intro's camera comes to him

**Block:** session 5, Sunday evening fixes before session-5q · **Status:** done

## Goal

The author: when the camera turns from the Publisher to the hero, the hero is not controllable for about a second;
he looks ahead and slowly draws the katana. The clip from the Meshy animation library; control starts when it ends.

## Decision

- The library has no sword draw or unsheathe. Three candidates bought (9 credits): Combat_Stance (a stance loop),
  Relax_arms_then_strike_battle_pose (starts and ends in a battle pose) and Sword_Shout. Only Sword_Shout starts from
  the idle stance: the blade hand at the hip (0.95 m) rises over the head and comes down to a guard pointing ahead.
- `hero_export.py` cuts its frames 0..92, plays them twice as fast (1.53 s) and moves the hips' 0.49 m step to the
  root (root-locked in game): `A_Hero_Draw`, in the hero's Entrance slot.
- `JamIntroRules::HeroDrawStart` (as the camera starts over to him) and `FullLength` (until the draw ends): the fight
  waits for it. A skip still ends everything at once.

## Test seam

`Jam.Intro.TheHeroDrawsAsTheCameraComesToHim` (red first). The look of the draw is not checked by eye.
