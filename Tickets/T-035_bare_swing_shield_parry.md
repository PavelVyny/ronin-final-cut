# T-035 Melee is always seen; Parry is a shield

**Block:** session 4 (playtest fixes) · **Status:** done

## Goal

The author's playtest: "Parry looks like a melee attack, while the melee attacks themselves have no animation. After
Parry is stolen I can still hit in melee, but with no visible swing, so it reads as a bug."

## What was in the way

- The blade sweep belonged to the hero's effects layer, and the first cut (`EffectsLostAt=1`) took it: from then on
  a swing showed nothing.
- The open parry window was a flat cyan disc on the floor around the hero: read from above as an area attack.

## Scope

- `JamFeedbackRules::LookOf(Type, Side, bHeroEffects)`: Neon, Bare or Hidden. Once the effects are cut, the hero's
  blade turns bare (a flat grey stand-in, the look of the unfinished game), never hidden; the dash trail and the
  spark of his hit on the Publisher still go. The Publisher keeps his neon. The rule replaces a condition in the view.
- `AJamEffectsView::DrawShield`: the parry window is a standing wall of 12 glowing panels round the body, as tall as
  the ronin, closing with the window (`ParryRingScale`).

## Test seam

`Jam.Feedback.MeleeIsAlwaysSeen`: red first (1 failing on a stub that always returned Neon).

## Done

146 `Jam.` tests green, 0 code check warnings. Clip `Clips/t035_bare_swing_shield.mp4` (45 s, bot): the shield cage
at 1, 9 and 9.5 s; after the first cut (Parry taken) the grey blade at 18, 21 and 30 s.
