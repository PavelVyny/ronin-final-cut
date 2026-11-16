# T-054 The dollars spill out of him

**Block:** session 5 · **Status:** done

## Goal

The author's playtest of `session-5c` (Sat 14 Nov 16:30): the paper dollars and the cash-stack heal never just
appear inside the Publisher; they jump out of his body along an arc and land a short distance away, as if they spill
out.

## Scope

- `JamFeedbackRules::Spill` (view rule): a bill leaves the edge of his body at an angle and lands its reach past the
  edge. `AJamMoneyView` throws every bill from the Publisher's body (all round him) instead of from the hit point;
  before, a 60 cm throw from his centre landed inside his 90 cm body. The arc is the existing pop of `BillPose`.
- `JamHealRules::Drop`: the cash stack flies from his body's edge towards where it lands. When the wall cuts the toss
  towards the hero short and it would land in him, it goes towards the middle of the arena instead (found by the new
  test).

## Test seam

`Jam.Feedback.BillsSpillOutOfHisBody`, `Jam.Heal.TheCashLandsClearOfHim` (red first: the stub, the old centre start
and the wall case, 60 cm from his centre). `Jam.Heal.AHitKnocksOutTheCashOnceItIsDue` now expects the edge start.

## Done

184 `Jam.` tests green, code check without warnings. Clip `Clips/t054_money_spills.mp4` (30 s, bot) recorded with
`HealDropInterval=8` (set back to 25): bills land in a ring around his body, the stack leaves his side on an arc.
