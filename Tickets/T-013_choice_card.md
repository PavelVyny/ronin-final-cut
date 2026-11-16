# T-013 The choice card and the slowdown

**Block:** C · **Status:** done

## Goal

When a steal starts the fight slows down for 3 seconds and a card shows the two mechanics. The player picks the one
to give away with A/D or the mouse. The card says what the Publisher will do with it.

## Scope

- The `Steal` phase scales the simulation time (slow motion, not a pause): fighters and projectiles crawl, nobody
  takes damage, the boss does not attack.
- Card in the HUD (canvas placeholder until Kirill's design): the Publisher's line, two options with name, key and
  what the boss does with it, a timer bar.
- Timeout: the Publisher takes the left option (default, see `QUESTIONS.md`).
- The take-all threshold shows a one-option card: "FINAL NOTE: cut the rest".
- All card strings in one place (`JamTexts`), in English.

## Test seam

`JamStealRules` in `Sim`:
- `TimeScale(Phase, Balance)`: `Jam.Steal.FightSlowsDuringTheChoice`.
- `TickChoice(StealState, ChoiceInput, RealDt)`: `Jam.Steal.ChoiceResolvesAtOnce`,
  `Jam.Steal.TimeoutGivesTheDefault`, `Jam.Steal.NoDamageDuringTheChoice`.

## Done

Playable in PIE in all three cameras (the cursor appears for the card and returns to the camera's mode). Tests green.

## As built

- **Deviation: the fight freezes during the card, no slow motion.** A and D are both the move keys and the answer
  keys, a frozen and cleared stage makes the checkpoint a plain copy, and the step stays deterministic. The card
  clock runs on simulation ticks (`StealChoiceSeconds`, `StealChoiceLock`). Slow motion can come back as a
  presentation effect in block G.
- A press during the first `StealChoiceLock` seconds is dropped, so a key held from the fight is not an answer.
- **No mouse choice in K1**: A or Left, D or Right. The cursor never appears, the camera modes stay as they are.
- The card does not say yet what the Publisher will do with each mechanic: one line per mechanic is left for
  `JamTexts` when the boss versions get their final shape.
- All HUD strings are in `Public/UI/JamTexts.h`.
- Tests: `Jam.Steal.*` (the card clock, the lock, the timeout, no damage during the card).
