# T-080 A backpedal plays the plain run backwards

**Block:** session 5, Sunday afternoon · **Status:** done

## Goal

Lead C's report on T-078: the library has no plain backwards run, so the back diagonals carried everything from 105
to 157.5 degrees off the facing, and at full speed they play about 3.5x: quick, skittery steps. The author's answer
(Sun 15 Nov): straight backpedalling uses the plain run played backwards.

## Decision

- The back diagonals now run only between 105 and 135 degrees (the hips turn back toward the move); from 135 on, the
  run plays backwards, so everything within 45 degrees of straight back is a calm backpedal with at most a 45 degree
  twist (30 at 150).
- The hip twist stays at 60 degrees; the Publisher's fast waddle and the steep bloom fall stay (the author's answers).

## Test seam

`Jam.Pose.ABackpedalPlaysTheRunBackwards` (red first: 150 degrees was the back diagonal);
`Jam.Pose.StrafingRunsTheDiagonalAndTwistsTheHips` checks the back diagonal at 120 instead of 135, where the edge is
now.
