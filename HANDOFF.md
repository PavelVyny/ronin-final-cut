# Session hand-off

Written Sat 14 Nov about 13:40 PC clock (UTC+2) at the end of the fourth jam session (with the author, 11:44 to
13:40). The next session starts from the prompt at the bottom. Earlier hand-offs are in git history (`7422eca`,
`b5cb811`, `db57452`). The report of this session: `Reports/session4_report.md`.

## Schedule change (the author, Sat 14 Nov 18:30, session 5)

- The Sat 22:00 feature freeze is **lifted**; work goes on late into Saturday night.
- **Every milestone ends with a green Shipping build pushed to itch**, so there is always something to submit.
- **New features stop Sun 12:00 PC clock** (the video capture needs the final look). Video Sun 18:00, submit Sun 20:00
  and never later than 21:59, unchanged.
- **Order now:** the fight block (T-055 the stolen mechanics at full strength, T-038 his Parry and Ultimate, T-056 a
  signature combo per stolen mechanic, T-057 the parry input buffer, T-058 the Ultimate turns during its wind-up), a
  Shipping build on itch; then the Meshy models T-025/T-026, then T-030, T-039. Details in `Tickets/INDEX.md`.

## Where we are

- **Clock:** see the schedule change above (the Sat 22:00 freeze below is history).
- **Last commits:** T-034 `c290c85`, T-035 `03b1e22`, T-036 `0bfada4`, T-037 `b2cdbf7`, T-041 `93a9b60`, sound
  `85aaee3`, then the hand-off commit. Working tree clean except `Art/Cover/` (the author's cover draft).
- **itch:** `pavyny/neon-ronin`, channel `windows`. `session-4a` build #2383584, `session-4b` build #2383825 (with the
  ElevenLabs sound), processing at push. Zip `Saved/Packaged/Pavyny_NeonRonin_Session4.zip`.
- **The page with every concept and sound:** https://claude.ai/artifact/JPCnbPCnQkjLDX57abegRW (tabs Concepts,
  Sounds). The HTML is in the session scratchpad; publish updates with the Artifact tool by this URL.

## What the game does today (154 `Jam.` tests green, code check without warnings)

- Top-down camera. Katana (LMB), shuriken (RMB), dash (Space), parry (Q), Ultimate (E). Four cuts and the FINAL NOTE;
  each strips a cosmetic layer and a music stem; at zero the stun gives everything back, the Ultimate wins.
- Aim: the hero model turns to the mouse every rendered frame; a neon hardware pointer (`Content/Slate/Cursors`).
- Teaching (`JamTeachRules`, `[JamTeach]`): "Q PARRY: SEND IT BACK" over the hero and a cyan ring on the shot about to
  land, until 3 deflects in the session; the Ultimate meter bar, a pulsing "E ULTIMATE READY" banner, a ring at the
  feet and a call over the head until it is used once; a chime when it fills.
- After the effects cut the hero's blade is a bare grey stand-in (never invisible); Parry is a standing shield cage.
- The Publisher is heavy (T-041): speed 260, radius 90, slow wind-ups, slam 34. Health 4800. Bot 60 of 60, median 151 s.
- Sound: ElevenLabs effects (14), four music layers at 120 BPM and the bare prototype loop, under Kirill's names.

## Decided at the end of the session (Sat about 13:45)

- **Boss:** `Art/Concepts/r4_boss_c2c_final.png`: the sumo executive with gold dollar-sign eyes, a gold tooth, an open
  cream suit, a plain gold tie, a dollar pin, cash in the pocket, gold knuckle guards. **Hero:** concept A.
- **The boss model is the session's job, through the Meshy API** (`MESHY_API_KEY` in `.env`, checked on Thursday);
  the author judges the result. One humanoid skeleton for both; the boss gets his own heavy clips.
- **New VFX (T-042), first in the order:** when hit, the Publisher bleeds paper dollars.
- **The next session runs with the author at the PC:** questions at the start, he plays builds between tickets.

## Open with people (`QUESTIONS.md`)

- **Kirill:** the brief was handed to the author (14 sfx names incl. `sfx_ultimate_ready_01`, 4 stems + `mus_poc_loop`,
  WAV 48 kHz, 120 BPM, 8-bar loops, deadline Sat 19:00 as proposed). Q5 his Drive folder. Our files stay unless his
  are better.
- Q3 the steal timer (default: the left option). The name RONIN: FINAL CUT is decided (Q1), not yet in the game.

## Remaining tickets, in this order (Q7: build everything)

1. **T-042** the Publisher bleeds paper dollars (about 1 h, stand-ins): a spray of small green bills from the hit
   point, the count by the damage (a rule in `JamFeedbackRules`, numbers in `[JamFeedback]`), they flutter, fall to
   the floor and fade; a big burst on the final blow. Part of the Publisher's look, so it never goes with the hero's
   effects layer. Test first, a clip.
2. **T-038** the Publisher uses the Parry and the Ultimate he stole (2 to 3 h: boss brain, telegraphs, bot table).
3. **T-025/T-026** the models through the Meshy API: boss from `r4_boss_c2c_final.png`, hero from `r1_hero_a.png`
   (image-to-3D, then rigging and animation if the API gives them; a tool `Tools/Art/meshy_models.py` like
   `fal_images.py`, with a ledger and a spend cap). Import, one skeleton, the heavy clips for the boss. Show the author
   each model before it goes into the game. Declare in `THIRD_PARTY.md`.
4. **T-040** rename to RONIN: FINAL CUT: `JamTexts::WinTitle` and every visible string; the itch page title is the
   author's (short).
5. **T-030** a short skippable intro before the fight (about 1 h).
6. **T-039** DLSS: check the plugin exists for UE 5.8 first; the first thing to drop.
6. Before the freeze: a person plays the `session-4b` build with sound; T-031 numbers by hand if the heavy boss is too
   easy for a person. Sunday: **T-032** submission (`THIRD_PARTY.md` is started).

## Known defects and gaps

- Aim feel, pointer and in-game sound mix untested by hands. The ready ring reads white, not magenta.
- The pad and the lead are a frequency split of one stem.
- `Jam.Bot.SlashesWhenTheBossIsInReach` was never seen red.
- `mus_poc_loop.wav` sits twice in `Audio/Eleven` (Sfx and Music): removing one needs the dry run and the author's yes.

## Working notes that cost time to find

- **itch push needs the author's explicit "push" in the current conversation**: the auto mode classifier blocks
  `itch_push.sh` otherwise. Pipe butler's output through `tr '\r' '\n' | grep -E "√|∙"`.
- A hardware cursor is a PNG under `Content/Slate`, set in `[/Script/Engine.UserInterfaceSettings] HardwareCursors`
  (DefaultEngine.ini); it is staged only with `+DirectoriesToAlwaysStageAsUFS=(Path="Slate")`.
- `demo_clip.sh` passes extra game switches through `JAM_DEMO_ARGS` (e.g. `-JamDemoHoldUltimate`). The bot presses E
  on the step the meter fills, so without it no clip shows the ready banner.
- To find a moment in a clip, scan `Saved/Demo/frames/f_*.png` with a small numpy script or tile them with ffmpeg
  (`tile=6x5`) and look.
- ElevenLabs: `Tools/Audio/eleven_audio.py credits|sfx|music|stems` with `--env F:/Unreal_Engine/UnrealProjects/Diskworld/.env`
  (the troll project's key; this project's `.env` has none). `wav_48000` works on the Creator plan for effects; music
  comes as MP3. Then `cut_loops.py`, `level_audio.py`, `import_music.sh Tools/Audio/import_eleven.json`.
- The import commandlet reimports every file in the JSON: all assets show as modified even when the WAV is the same.
- fal: `--max-usd` caps the total in the ledger, not the round (1.70 USD booked so far).
- DEVLOG times: take them from `date`, not from memory; two entries of this session ran ahead and were put back.
- From earlier sessions: `AActor` has a member `Layers`; unity build needs unique names in anonymous namespaces;
  multi-line commit messages through `git commit -F <file>`, no heredocs.

## Prompt for the next session

```
Read F:\Unreal_Engine\UnrealProjects\MegaJam2026\HANDOFF.md, then CLAUDE.md, .claude/rules/cpp.md, QUESTIONS.md,
Reports/session4_report.md and the "Standing rules" of JAM_START_PROMPT.md.

Check the clock first and say how long it is to the feature freeze (Sat 22:00 PC clock). I am at the PC for this
session. Before any code, ask me with AskUserQuestion, in one round, what you need: how the dollar bleed should look
(T-042), what the Publisher does with my stolen Parry and Ultimate (T-038), the Meshy spend cap and whether to rig
and animate through Meshy (T-025/T-026), whether Kirill's files came (Q5). Show options with your recommendation first.

Then work the "Remaining tickets" of HANDOFF.md in order: T-042, T-038, the Meshy models, T-040, T-030, T-039.
Test first, one commit per ticket, build and tests, a short clip and a DEVLOG.md entry after each. Show me each Meshy
model before it goes into the game. Warn in chat before any game window. After each ticket that changes the feel, offer
a Shipping build for me to play; push to itch only on my "push". Never cross the feature freeze; stop starting new
tickets at 21:00, then a Shipping build, a smoke run, HANDOFF.md and Reports/session5_report.md.
```
