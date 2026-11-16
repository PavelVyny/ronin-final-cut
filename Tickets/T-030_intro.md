# T-030 A short intro before the fight

**Block:** session 5, models · **Status:** done

## Goal

Q7 (Sat 14 Nov): a short intro before the fight, skippable. The author, Sun 15 Nov 00:30: the Publisher's
`Kung_Fu_Punch` clip (7.3 s of spins and kicks) goes there as his entrance show-off, not into the fight.

## Decision

- Before the first fight of a launch the simulation waits (the game mode does not advance it). The Publisher plays
  `Kung_Fu_Punch` whole at its own speed (`EJamClip::Entrance`, root locked so he spins in place), the camera
  watches him, a title card sits low on the screen ("THE PUBLISHER", "He has notes on your game. He will take what he
  likes.", "LMB / SPACE skip"), and over the last 0.8 s the camera goes over to the hero. Then the fight starts.
- LMB, Space, Enter or Escape skip it, but not in its first half second (a click still held from starting the game).
  A skip fades his show-off out over one blend.
- Once per launch: a retry or a new fight after a win starts at once.
- On the way: the parry shield was drawn for the step before the first one (an untouched parry state reads as
  raising); one frame before, the whole intro now. The effects view draws no shield before the first step.

## Test seam

`Jam.Intro.EndsAfterItsShowOrOnASkip`, `Jam.Intro.TheTitleFadesInAndOut`,
`Jam.Intro.TheCameraGoesOverToTheHeroAtTheEnd` (red first against stubs). The look: `Clips/t030_intro.mp4`.
