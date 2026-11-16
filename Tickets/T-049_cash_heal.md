# T-049 The Publisher drops a cash-stack heal

**Block:** session 5 · **Status:** done

## Goal

The author's playtest of `session-5a` (Sat 14 Nov 15:05): every so often the Publisher drops a heal made of dollars
that reads clearly as a pickup, not as more paper on the floor. Touching it heals the hero, with a visible heal effect
and a cash register "ka-ching". Answers in chat: 25 health every 25 s, a glowing cash stack, knocked out by a hit.

## Scope

- Simulation, `JamHealRules`: once a drop is due (25 s after the start, a retry or the last drop), the next hit on the
  Publisher knocks a stack out; it flies 250 cm towards the hero in 0.5 s (kept inside the arena), lies 10 s, at most
  1 on the floor. The hurt hero takes a landed stack by touch (his body plus 40 cm): +25, never past full; a whole
  hero leaves it for later. Events `HealDropped`, `HealTaken`. Numbers in `[JamBalance]`.
- The bot walks to a landed stack once it has lost at least 25; any dodge still comes first.
- View: a 90 by 50 by 40 cm green stack with a gold band floating 80 cm up, bobbing and turning, over a pulsing dashed
  green ring; it blinks in its last 2 s. Taken: two green rings climb the hero. The dollar bleed (T-042) is unchanged.
- Sound: `sfx_heal_cash_01` from ElevenLabs (our prompt in `Tools/Audio/eleven_sfx.json`), levelled to -18 dB,
  imported with `Tools/Audio/import_heal.json`; cue 15 on Kirill's list (`QUESTIONS.md` Q8, T-028).

## Test seam

`Jam.Heal.AHitKnocksOutTheCashOnceItIsDue`, `Jam.Heal.TheHeroHealsOnTouch`, `Jam.Heal.TheCashFadesAndStaysInTheArena`,
`Jam.Sound.TheHealRingsTheTill`. Red first on stubs (drop checks failing, the take test crashing on the missing stack).

## Bot table (20 seeds per policy)

60 of 60 won, median 92 s (59 to 185), unchanged; the closest win went from 16 to 43 health. The fight length stays
in range.

## Done

168 `Jam.` tests green. Clip `Clips/t049_cash_heal.mp4` (30 s, bot) recorded with `HealDropInterval=8` (set back to
25 before the commit) so a drop shows early: the stack lands at about 10 s. The bot was whole then, so the clip shows
no pickup; the first 45 s try at 25 s showed none at all (the bot died at 24 s and the retry restarts the timer). The
stack was made 1.6 times bigger after the first clip: at 56 cm it was barely bigger than a bill from the camera.
