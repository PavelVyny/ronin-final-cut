# T-079 The Publisher's feet keep up with him

**Block:** session 5, Sunday feedback · **Status:** done

## Goal

The Publisher slides: he moves at 260 cm/s, six times what his walk covers (43 cm/s), and the walk played at most five
times as fast (T-066). His feet have to match his speed.

## Decision

- **The walk plays as fast as his feet need:** the cap goes from 5 to 6.5 (he needs 6.05), through the same
  `JamPoseRules::StrideRate` as the hero's runs (T-078). The feet no longer slide.
- **His upper body takes a third of the walk's sway** (`UpperWalk` 0.35 on his figure, the rest is his idle): played
  six times as fast the orc walk's arm and shoulder swing read as a flail.
- He also gets the T-078 hip twist toward the move (no diagonals: the run forwards turns up to 60 degrees, backwards
  beyond 110) and the lean into turns.

## Test seam

`Jam.Pose.TheRunKeepsUpWithTheGround` (T-078) covers the rate. The sway is presentation, checked in the stride
gallery's last part (`-JamStrideGallery`, from 20 s).
