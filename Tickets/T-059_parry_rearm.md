# T-059 A good parry is ready again at once

**Block:** session 5, fight · **Status:** done

## Goal

The author's playtest of `session-5i` (Sat 14 Nov 19:50): spamming LMB into a burst of shots sent back every other
shot. The 0.8 s cooldown after a parry let the next shot through. Asked: "every shot in reach is deflected back";
remove the cooldown or reset it on a successful parry; the T-057 input buffer stays.

## Decision

A parry that sends a shot back is ready again at once (the cooldown resets on the deflect). A parry that sends nothing
back keeps its 0.8 s cooldown, so a swing into empty air is not a free shield. Only the hero: the Publisher's stolen
shield stance (T-038) keeps its own 4 s cooldown.

## Test seam

`Jam.Parry.SpamSendsBackEveryShotOfABurst` (red first: 1 of 4 memos back), `Jam.Parry.AParryThatSendsNothingBackKeepsItsCooldown`
(guard).

## As built

`JamParryRules::Rearm`, called from `AdvanceProjectiles` on the hero's deflect.

## Done

207 `Jam.` tests green, code check without warnings. Bot table unchanged (60 of 60, median 99 s, closest win at
26 health): the bot clicks only within 0.15 s of a hit and never spams, so it does not measure this change; the
author's hands do.
