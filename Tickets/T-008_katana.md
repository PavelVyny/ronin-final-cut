# T-008 Katana

**Block:** B · **Status:** done

## Goal

LMB swings a series of three strikes towards the aim. Each strike hits everything hostile in an arc in front.

## Scope

- Combo: a strike has a wind-up, a hit moment and a recovery; pressing again inside the chain window continues the
  series, the third strike is the heaviest, a late press starts over.
- Arc hit test on the plane against the enemy circle. A short step forward on each strike.
- Numbers (damage per strike, range, arc, timings) in the balance section.
- View: a flat arc flash placeholder on the hit moment.

## Test seam

`JamKatanaRules` in `Sim`:
- `InArc(AttackerPos, AimDir, TargetPos, TargetRadius, Range, HalfAngle)`: `Jam.Katana.HitsInFront`,
  `Jam.Katana.MissesBehind`, `Jam.Katana.TargetRadiusCounts`.
- `NextStrike(ComboState, Now, Balance)`: `Jam.Katana.ThreeStrikesThenRestart`,
  `Jam.Katana.LatePressStartsOver`, `Jam.Katana.CannotStrikeDuringWindUp`.
- `Jam.Fight.KatanaDamagesOncePerStrike`.

## Done

The series is playable in PIE against the dummy. Tests green.
