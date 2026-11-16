# T-046 The Ultimate shows where and when it lands

**Block:** session 5 · **Status:** done

## Goal

The author's playtest (Sat 14 Nov 14:20): "the Ultimate is hard to use, I could not understand how it worked even when
I used it twice; show where it lands and when".

## Scope

- `JamFeedbackRules::UltimateLane`: shown while the fighter owns a full meter or winds up. Before the press it follows
  the aim; after it, it is locked on the pressed way and fills from 0 to 1 over the wind-up, reaching 1 on the step
  the cut lands. `bOnTarget` when the other fighter stands in it.
- `AJamEffectsView::DrawLane`: the lane's edges on the floor (1600 by 180 cm), faint while ready and aimed at the air,
  full magenta when the Publisher is in it or it is pressed; a fill runs from the feet to its end. The Publisher's
  lane (his stolen Ultimate, T-038) shows in his red, only while he winds up.

## Test seam

`Jam.Feedback.TheUltimateShowsWhereAndWhen`: hidden on an empty or stolen meter, along the aim when ready, lit with
the boss in it, locked on the pressed way, fill 0, about half, 1. Red first on a stub (7 checks).

## Done

162 `Jam.` tests green. Clip `Clips/t046_ultimate_lane.mp4` (30 s, bot): the lane fills through the Publisher at
about 10 to 12 s.
