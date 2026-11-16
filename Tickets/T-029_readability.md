# T-029 Readable attacks: hit feedback, camera shake, the explosive in his hands

**Block:** G · **Status:** done (on stand-ins; the real VFX come with T-027 and the art)

## Goal

A person, not only the bot, can read the fight from the top-down camera: what hits and how hard, when the parry
window closes, and which of the Publisher's shuriken will blow up (the finding of T-033).

## Scope

- `JamFeedbackRules` (`View`, pure), numbers in `[JamFeedback]` of `DefaultGame.ini`:
  - `ShakeKick`: the slam, the Ultimate slash, an explosion and the hero hurt shake the camera; swings, dashes and
    hits on the boss do not. `ShakeLeft` dies out squared over `ShakeSeconds`, `ShakeOffset` never leaves the
    amplitude. The rig keeps the stronger of a running shake and a new kick.
  - `ShowsBlastRing`: an explosive shuriken of the Publisher's carries a dashed ring of its blast radius on the
    ground; the hero's own, or one he parried back, does not.
  - `SparkRadius`: a ring around the body that was hit, sized by the damage between two limits.
  - `FlashWeight`: the hurt hero flashes fully white; the Publisher only by the size of the blow (a ball tints him,
    the katana finisher flashes him): "the boss flashes white a lot" is gone.
  - `ParryRingScale`: the parry ring opens wide and closes with the window, so the time left is seen.
- Views: the explosive shuriken is yellow and pulses (orange was too close to his red); the parry ring is brighter.
  The spark of a hit on the Publisher goes with the hero's effects layer when it is cut.

## Test seam

`Jam.Feedback.OnlyHeavyBlowsShakeTheCamera`, `ShakeDiesOut`, `TheBossExplosiveShowsItsBlast`,
`HitsShowTheirWeight`, `ParryRingClosesWithTheWindow`, `IniIsComplete`. Red first: 6 failing with stub rules.

## Done

134 `Jam.` tests green, 0 code check warnings. Clip `Clips/t029_readability.mp4` (30 s, bot): sparks on hits, the
yellow explosive. The blast ring needs a stolen shuriken and shows in the whole-fight clip of the session.
