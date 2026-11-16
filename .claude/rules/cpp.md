---
paths:
  - "Source/**"
---

# C++ standard of the jam project

Ported from the CastleFight and Diskworld standards before the jam start. It holds working conventions only: no
game design. A new rule is added here only by the author's decision.

## Style

- Epic C++ Coding Standard + `.clang-format` in the root: tabs, Allman, braces always, `ColumnLimit: 140`.
  `check_code.py` checks the format, never align by hand.
- Comments and commit messages in English only.
- A comment describes the behaviour that exists, not the intended one. Change the behaviour, fix the comment in the
  same commit.
- Epic prefixes: `b` for bool, `F/U/A/E/I` for types.

## Decomposition

- A decision (who is hit, how much damage, what a rule allows, when the run ends) lives in a pure
  `namespace <Feature>Rules` with no world and no actors: `Public/<System>/<Feature>Rules.h`, free functions with
  `JAMGAME_API`, and a header line `Tests: Jam.<Area>.*`.
- An actor, component or subsystem only glues: collects inputs, calls the rule, applies the result.
- A big class is split into `.cpp` files by responsibility with one shared `.h`.
- Soft thresholds: a `.cpp` over ~600 lines, a function over ~60, nesting over 3. Exceeding one is a signal, not a
  ban: split or justify.

## Systems and dependencies

- Folders `Public/<System>` and `Private/<System>`, plus `Tests`.
- **The simulation does not know about the presentation.** `Core` and `Sim` never include `View`, `UI` or `Audio`.
  The directions are forbidden in the `check_code.py` ratchet (`SIMULATION`, `PRESENTATION`): extend both lists when
  a new system folder appears.
- Balance numbers (prices, health, damage, timings) live in data, not in code.
- Randomness of the simulation comes from a seeded stream, not `FMath::Rand`, so a run can be replayed in a test.

## Logs

- No `LogTemp` in new code. Use the category of the system or `DEFINE_LOG_CATEGORY_STATIC` for a single file.
- Warning and Error only for a breakage or a wrong setup; state changes are `Verbose`, per-tick traces `VeryVerbose`.

## Tests

- Every new decision in `*Rules` gets a test `Jam.<Area>.<Case>` before the code (test first). Glue with the engine
  is checked in PIE, a test is optional.
- The number of `Jam.` tests only grows; a failing test means reverting the change, not bending the test to the code.
- A bug is fixed separately from a refactoring, test first.

## Blueprint seam

- A new Blueprint exposure is opened when needed, not "just in case".
- Logic is not duplicated between an EventGraph and C++: the decision is in C++, the Blueprint only calls.
- Renaming a `UCLASS`/`UPROPERTY`/`UFUNCTION` visible in Blueprints only with CoreRedirects.
- Deletions (C++, assets) only through a dry run with counts and the author's explicit yes.

## Done (end of a task with code)

1. `bash Tools/Build/build_and_test.sh --open` is green: build, `Jam.` tests without failures, checks. The agent
   closes the editor and runs the full build itself, without asking.
2. Every new `check_code.py` warning is fixed or justified in one line of the report.
3. New decisions are covered by `Jam.` tests, their number did not drop.
4. A git commit in English.
