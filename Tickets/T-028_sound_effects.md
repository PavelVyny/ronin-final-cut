# T-028 Music and sounds from Kirill (part: stub sound effects)

**Block:** G · **Status:** doing (ElevenLabs sound and music in the game since Sat 13:22; Kirill's files, if they come, and the Figma UI still open, Q5)

## Sat 14 Nov 13:22: ElevenLabs sound in the game

The author asked for the sound designer's task done with ElevenLabs, as an example for Kirill and the final sound if
his files do not come. `Tools/Audio/eleven_audio.py` (key from a `.env`, never printed, every request in
`Audio/Eleven/ledger.json`), prompts in `eleven_sfx.json` and `eleven_music.json`; `cut_loops.py` cuts 8 bars at
120 BPM from the same downbeat (8.085 s) in every stem, splits the mixed synth stem into pad and lead, crossfades the
seam; `level_audio.py` sets each file's loudness by role; `import_eleven.json` imports over the stubs under the same
names. Listen on the Sounds tab of https://claude.ai/artifact/JPCnbPCnQkjLDX57abegRW. Declared in `THIRD_PARTY.md`.

## Goal

Every action the player has to hear makes a sound: the twelve sounds of section 4 of `JAM_PLAN.md` plus the slam.
Stubs now, Kirill's files later under the same names, with no code change.

## Scope (this part)

- `JamSoundRules` (`Audio`, pure): `CuesOf(Events, Fight)` picks the cues of a frame's events, each cue once per
  frame in the order of first appearance. The katana (both sides), a shuriken throw (not memos, not the ball), an
  explosion, the slam, a dash, a deflection (the parry), the Ultimate slash, a hit on the boss, the hero hurt, the card
  opening, a mechanic taken, the stun giving everything back, and the winning slash as the final blow.
- `NameOf(Cue)`: `sfx_<what>_01`, the naming of Kirill's delivery.
- `AJamSounds` (`Audio`): one `USoundWave` per cue from `/Game/Audio/Sfx`, played 2D; the game mode calls it after
  the views.
- Stub sounds made in the jam by `Tools/Audio/make_stub_sfx.py` (synthesised, no samples), WAVs in `Audio/Stub/Sfx`,
  imported by `bash Tools/Audio/import_music.sh <root>/Tools/Audio/import_sfx.json`.

## Test seam

`Jam.Sound.EveryActionIsHeard`, `OnlyTheShurikenIsThrownAloud`, `ACueSoundsOncePerFrame`,
`TheFinalBlowHasItsOwnSound`, `NamesFollowTheDelivery`. Red first: 5 failing with stub rules.

## Done (this part)

139 `Jam.` tests green, 0 code check warnings. In game, `au.Debug.Sounds 1` lists `sfx_boss_hit_01` and
`sfx_shuriken_throw_01` beside the five music tracks: `Clips/t028_active_sounds.png`, 15 s `Clips/t028_sfx_debug.mp4`
(frames only, no sound track).

## Replacing the stubs

Put Kirill's WAVs into `Audio/Kirill/Sfx/` under the same names, copy `Tools/Audio/import_sfx.json` with those paths,
run the import script with it. No code change.

## Added later

- T-037: `sfx_ultimate_ready_01`, a short rising chime when the hero's Ultimate meter fills.
- T-049: `sfx_heal_cash_01`, a cash register "ka-ching" when the hero takes the Publisher's cash-stack heal.
