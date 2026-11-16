# T-083 The Publisher's health bar fades in when the fight starts

**Block:** session 5, Sunday quick fixes before session-5p · **Status:** done

## Goal

The author: the Publisher's HP bar must not show during the intro; it fades in smoothly when the fight starts.

## Decision

`JamIntroRules::BossBarAlpha(SinceFight, Fade)`: 0 through the intro, then up to 1 over 0.6 s. The game mode notes
when the fight starts after the intro; the HUD draws the bar and his numbers at that alpha.

## Test seam

`Jam.Intro.TheBossBarFadesInWhenTheFightStarts` (red first).
