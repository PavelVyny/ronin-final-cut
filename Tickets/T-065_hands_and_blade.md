# T-065 The katana in the hand, throws from the other hand

**Block:** session 5, polish · **Status:** done

## Goal

The author, Sun 15 Nov 02:40: where is the katana while the hero idles, throws, runs? It should be in the hand, in
the right place. Do shurikens leave the hand that throws them? (Plan: `PLAN_polish_night.md`, step 1.)

## Decision

- **The grip is measured, not guessed.** A pose gallery (`-JamPoseGallery`: the fight stands, the hero holds twelve
  poses in turn, the camera close at his right and left front) logs the hand bone, the fingertip end and his facing.
  `hand_r`'s +Y runs back to the elbow, so the fingers lie along -Y; with the arm hanging the thumb is at -Z. The
  blade (the model's -X) points along -Z, its grip's middle sits 8 cm down the fingers, in the fist:
  `FRotator(90, 0, -90)`, `(0, -8, -17.9)`, scale 0.85. Checked in all twelve poses. `-JamKatana=X,Y,Z,P,Y,R` tries a
  grip without a build (not in Shipping).
- **The hero throws with his left hand**, the katana stays in the right (the author's pick). Two Meshy library clips
  were bought (6 credits, 88 of 250 used): the quick one-arm cast is mirrored onto the left arm in the export
  (`hero_export.py`, `Throw_L`, the left hand fastest at 0.57 s); the slow two-hand throw is not used.
- **His shurikens and balls start a throw** (`JamPoseRules::CueFor`, the hero's `ProjectileSpawned`), shown from the
  release on, so the arm is at the release when the shot appears. A strike in the same step still wins.
- **Shots leave the hand.** The projectile view draws a new shot at its thrower's hand (the hero's left, the
  Publisher's right) and eases it onto its flight line within 0.08 s; the simulation still throws from the body.

## Test seam

`Jam.Pose.HisThrowsSwingTheHeavyBlow` now also asks that the hero's ball starts a throw from the release, and that a
strike of the same step wins. The test was written with the rule change, without a separate red run.
