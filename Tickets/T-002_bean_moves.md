# T-002 The bean moves in the arena

**Block:** A · **Status:** done

## Goal

WASD moves a bean around the round arena; it cannot leave it.

## Scope

- `FJamFighter` in the sim: position, facing, radius, speed. `FJamIntent`: move vector, aim, buttons.
- The move step: velocity from the intent, the position clamped to the arena circle.
- `AJamPlayerController` reads the keys into an intent (keys are polled: a dozen fixed keys, no input assets to cook).
- `AJamFighterView`: an actor with a capsule mesh that follows its sim fighter (smoothed between steps).

## Test seam

`JamMoveRules` in `Sim`:
- `Velocity(MoveInput, Speed)`: a diagonal is not faster than a straight line. `Jam.Move.DiagonalIsNotFaster`.
- `ClampToArena(Position, Radius, ArenaRadius)`: `Jam.Move.StaysInsideArena`, `Jam.Move.SlidesAlongTheEdge`.
- `WorldMove(Input2D, BasisYaw)`: stick input turned by the camera basis. `Jam.Move.InputFollowsCameraBasis`.

## Done

The bean walks in PIE and stops at the arena edge. Tests green.
