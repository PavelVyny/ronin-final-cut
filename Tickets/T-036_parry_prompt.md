# T-036 The game teaches Parry before the Publisher takes it

**Block:** session 4 (playtest fixes) · **Status:** done

## Goal

The author's playtest: "The player only learns that enemy projectiles can be deflected back at the boss when the
Publisher steals Parry. It has to be obvious before that." Option taken in chat: a context prompt.

## Scope

- `JamTeachRules` (`UI`, pure), numbers in `[JamTeach]` of `DefaultGame.ini`:
  - `ParryPromptFor`: the enemy shot that lands on the hero first within `ParryPromptLead` (0.6 s), while the fight
    runs, the hero owns Parry, Q would open it now and fewer than `ParryLessons` (3) deflects are behind him.
  - `Learn`: counts the hero's deflects from the step events. The count lives in the game mode for the session:
    a retry or a new fight does not teach again.
- Views: "Q  PARRY: SEND IT BACK" over the hero's head (`AJamHUD::DrawOverHero`), a cyan dashed ring round the shot
  it is about.

## Test seam

`Jam.Teach.ParryPromptMarksTheShotAboutToLand`, `ParryPromptOnlyWhenQWorks`, `ThreeDeflectsEndTheParryLessons`,
`IniIsComplete`: red first (4 failing on stubs).

## Done

150 `Jam.` tests green, 0 code check warnings. Clip `Clips/t036_parry_prompt.mp4` (20 s, bot): at 0.8 s the prompt
over the hero and the cyan ring round the Publisher's incoming shot; the bot parries at once, so it shows briefly.
