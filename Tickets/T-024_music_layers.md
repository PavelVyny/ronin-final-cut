# T-024 Music stems as layers

**Block:** E · **Status:** done (stub stems; Kirill's files replace them under the same names)

## Goal

The music degrades with the game: four stems play together and each cut fades one out, until only the
proof-of-concept metronome is left; the stun brings the whole band back.

## Scope

- `JamLayerRules::StemsFor(CutsDone, Phase, Settings)`: how many stems play. `StemsLeft=(4,3,2,0)` in `[JamLayers]`:
  the lead goes at cut 1, the pad at cut 2, bass and drums at the final cut. With four cuts (T-021) one more line.
- `AJamMusic` (`Audio`): five looping tracks start together and keep running (silent play keeps them on one beat);
  a change only fades volumes over 0.6 s.
- Stub stems made in the jam by `Tools/Audio/make_stub_stems.py` (synthesised, no samples): 120 BPM, 8 bars,
  48 kHz, named like Kirill's delivery (`mus_main_1_drums` ... `mus_main_4_lead`, `mus_poc_loop`). WAVs in
  `Audio/Stub`, imported to `/Game/Audio/Music` by `Tools/Audio/import_music.sh` (ImportAssets commandlet).

## Test seam

`Jam.Layers.MusicLosesAStemPerCut`, `Jam.Layers.MusicReturnsAtTheStun`, `Jam.Layers.IniMusicEndsInTheMetronome`.
Red first with a stub rule (2 failing), then green.

## Done

Build and 115 `Jam.` tests green, 0 code check warnings. In game, `au.Debug.Sounds` lists all five tracks active:
`Clips/t024_active_sounds.png`. The bot clips are captured as frames, so they carry no sound; the music is heard in
the package.

## Replacing the stubs

Put Kirill's WAVs into `Audio/Kirill/` under the same names, copy `Tools/Audio/import_music.json` with those paths,
run `bash Tools/Audio/import_music.sh <that json>`. No code change.
