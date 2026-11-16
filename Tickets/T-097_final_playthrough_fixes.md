# T-097: the author's final play-through fixes

Status: done (Sun 15 Nov 2026, session-5v)

## Asked

1. The comic's first panel: about 2 s of typing before the phone rings, keeping the soft push-in.
2. A short glass shatter exactly as the panel of the Publisher smashing through the screen appears (ElevenLabs).
3. The intro: the Publisher turned a further 10 degrees clockwise, seen from the top-down camera, in his show.
4. The intro's music: the theme fades in quietly under his kung fu and his line and is full exactly as the camera goes
   to the hero; on a skip it fades in from the skip.
5. The Publisher imported again with the shirt fix (f3a5269) before packaging.

## Done

1. Panel 1 is 2 s; its push-in eased to 1.15 so it stays soft over the shorter time.
2. `sfx_comic_shatter_01` (ElevenLabs, `Tools/Audio/eleven_comic_sfx.json`, ledger), trimmed to 40 ms before its hit
   (`Tools/Audio/trim_onset.py`; the hit was 0.79 s in), on panel 4 as `EJamComicSound::Shatter`.
3. The Publisher's `EntranceTurn` -120 to -110 (negative is counter-clockwise from above).
4. `JamIntroRules::IntroTheme`: the theme's share rises squared from 0 at the show's start to 1 at `HeroDrawStart`
   (the camera's move); after a skip the usual crossfade takes it up from where it is. The line-over cue is gone.
5. Done in T-096 (c7e9cc4): the mesh, the clips and the repainted base colour.

Tests: `Jam.Comic.TheFirstPanelPushesInSlowly`, `Jam.Comic.TheRingComesBeforeTheCall`, `Jam.Intro.TheThemeRisesUnderHisShow`
(it replaces the halfway cue test of T-096), `Jam.Intro.TheAmbientGivesWayToTheTheme`. Not watched in a game window:
the author checks in play.
