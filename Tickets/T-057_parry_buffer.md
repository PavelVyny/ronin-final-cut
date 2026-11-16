# T-057 An early parry click waits for the shot

**Block:** session 5, fight · **Status:** done

## Goal

The author's playtest: a shot is about to hit, the click comes, it hits anyway. Cause: the strike parries (T-044) only
when a shot reaches the hero within the 0.25 s window at the moment of the click; a click earlier than that becomes a
swing. The author's answer (Sat 14 Nov 18:30, "look-ahead"):

- A click (LMB or Q) when a shot will reach the hero within `ParryBuffer` + `ParryWindow` (0.3 + 0.25 s) does not
  swing: it is held, and the parry opens on the step the shot comes within the window.
- Otherwise LMB swings as now; Q with no shot coming opens the window at once as now.
- The 0.8 s cooldown is kept; a held click that sees the shot go away (dodged, gone) lapses after `ParryBuffer`.

## Test seam

`Jam.Parry.EarlyClickWaitsForTheShot`, `Jam.Parry.EarlyClickDoesNotSwing`, `Jam.Parry.HeldClickLapses`,
`Jam.Parry.ClickWithNothingComingSwings` (red first).

## As built

- `ParryBuffer=0.3` in `[JamBalance]`, `FJamParryState::HeldUntilTick`. In `UseButtons` the hero's strike, when no
  shot is within the window but one is within buffer + window, is held: no swing; while held, the parry opens on the
  step the shot comes within the window. The hold lapses after the buffer and spends nothing.
- Q: the hero has had no parry key since T-044 (LMB parries), so the buffer is on LMB only.
- **A second cause found on the way:** the window opens after the projectiles of its step have flown, so a window of
  N steps guards only N - 1 flights, while `ShotInReach` looked N flights ahead: a shot that touched on the last of
  them hit through an open parry. `ShotInReach` now looks N - 1 steps ahead.

## Test seam (as built)

`Jam.Parry.EarlyClickWaitsForTheShot` (red first: first on the missing hold, then on the off-by-one above: parry at
step 8, hit at step 20 with the window closing at 20), `Jam.Parry.HeldClickLapses`, `Jam.Parry.ClickWithNothingComingSwings`
(guards, green from the start). The "no swing" case is inside the first test.

## Done

203 `Jam.` tests green, code check without warnings, bot table unchanged (60 of 60, median 97 s). No clip: the bot
only clicks within 0.15 s of a hit, so it never uses the buffer; the author's hands are the check.
