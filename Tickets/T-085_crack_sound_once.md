# T-085 The crack's sound plays once, short and quiet; a thinner line

**Block:** session 5, Sunday quick fixes before session-5p · **Status:** done

## Goal

The author: the degradation crack's sound plays twice and too loud, and clashes with the voice line; play it once,
much shorter and quieter, ducked under the voice. The crack line on screen is too thick and too neon.

## Decision

- Why it played more than once: while the crack holds the fight, the simulation does not step, so the last frame's
  events (the steal) stayed in the frame and every view replayed them each frame of the hold. A held frame now clears
  them (`UJamFightSubsystem::Hold`).
- `JamSoundRules::MixOf`: the steal's sound plays at 0.3, at 0.12 while a voice line plays, and fades out after
  0.35 s of its 1.2 s. The voice hears the frame before the sounds play, so a line that starts with the cut ducks it.
- The crack line: a faint 6 px halo, a 2.5 px coloured body and a hairline core (was 14/5/2 px and bright); one line
  while it runs, a small head mark.

## Test seam

`Jam.Sound.TheCrackIsShortQuietAndDucksUnderTheVoice` (red first). The replay fix is engine glue, checked in a
capture.
