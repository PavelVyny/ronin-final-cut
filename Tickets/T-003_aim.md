# T-003 Aim abstraction

**Block:** A · **Status:** done

## Goal

The simulation receives one thing, an aim (a direction on the arena plane and a target point), whatever camera is
active. The hero faces the aim.

## Scope

- `FJamAim`: unit direction in 2D and the aimed point.
- Two aim sources: a cursor ray onto the arena plane (top-down), and the view yaw (third and first person).
- The controller picks the source from the active camera mode and writes the aim into the intent.

## Test seam

`JamAimRules` in `Sim`:
- `FromRay(RayOrigin, RayDir, PlaneZ, FighterPos)`: `Jam.Aim.CursorRayHitsThePlane`,
  `Jam.Aim.RayParallelToThePlaneKeepsLastAim`, `Jam.Aim.CursorOnTheFighterKeepsLastAim`.
- `FromYaw(YawDegrees, FighterPos, Reach)`: `Jam.Aim.YawGivesDirection`.

## Done

In PIE the bean turns to the cursor in top-down and to the view direction in the other two cameras. Tests green.
