# T-048 A retry keeps the health of the cut

**Block:** session 5 · **Status:** done

## Goal

The author's playtest of `session-5a` (Sat 14 Nov 15:05): coming back from a checkpoint with full health felt
unfair. The checkpoint stores the hero's health at the moment the steal resolves; a retry brings back exactly that,
except at 20% or less, when he comes back with 50%.

## Scope

- `JamCheckpointRules::RetryHealth`, used by `Capture` (taken at the fight's start and on each resolved steal).
- `[JamBalance]` `RetryLowHealthShare=0.2`, `RetryLowHealthRestore=0.5`.

## Test seam

New `Jam.Retry.HeroComesBackWithTheHealthOfTheCut` (100, 60, 21, 20, 3, a fresh fight); red first (5 checks).
`Jam.Retry.HeroIsHealedAndKeepsOnlyUnstolenMechanics` lost its "whole again" check (now in the new test) and became
`Jam.Retry.HeroKeepsOnlyUnstolenMechanics`.

## Done

164 `Jam.` tests green; the bot table did not move (the bot never dies). No clip: the bot never retries.
