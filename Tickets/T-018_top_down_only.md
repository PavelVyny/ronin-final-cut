# T-018 Only the top-down camera stays

**Block:** D · **Status:** done

## Goal

The author chose camera 0, top-down 3/4 (Fri 13 Nov). The shoulder and first-person cameras are switched off, so
every later ticket (art, animations, readability) works for one view only.

## Scope

- The player controller no longer reads F1, F2 and F3 and has no mouse-look branch: the cursor aims, the keys move
  in screen axes.
- The demo run plays on the top-down camera only; `-JamDemoCamera` is gone (`demo_clip.sh` ignores its argument).
- The HUD loses the camera hint and the crosshair.
- On the way, a K1 defect on the same lines: the debug keys F5 and F6 and their hint are compiled out of Shipping.
- Kept: `JamCameraRules` with all three modes and their tests. The rules are pure and cost nothing; removing them
  would drop tests (`.claude/rules/cpp.md`).

## Test seam

Glue only, no new decision: the top-down view and its move basis are covered by `Jam.Camera.TopDownLooksAtTheHero`
and `Jam.Camera.TopDownMovesInScreenAxes`. Checked by a bot clip.

## Done

Build and 107 `Jam.` tests green, 0 code check warnings. Clip `Clips/t018_topdown.mp4`.
