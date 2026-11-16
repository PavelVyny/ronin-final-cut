# Dev log: RONIN: FINAL CUT (working title NEON RONIN until Sat 14 Nov)

Epic AI MegaJam 2026. Times are the home PC clock (UTC+2). The full agent transcripts are in `_AgentLogs/`.

## AI tools used

| Tool | Used for |
|---|---|
| Claude Code (Anthropic) | All code (test first), tickets, builds, packaging, asset import, this log |
| Unreal Engine 5.8 MCP toolsets | The agent drives the editor: levels, assets, PIE |
| fal: OpenAI GPT Image 2.6 (`openai/gpt-image-2.6/flare/text-to-image`) | Concept images of the hero, the boss, the arena and the weapons. Prompts and spend: `Art/Concepts/ledger.json` |
| ElevenLabs (sound effects, music, stem split) | The game's sound effects and music layers (session 4). Prompts: `Tools/Audio/eleven_sfx.json` |

Tools are added to this table at the moment they are first used.

## Thu 12 Nov

### 13:35 Jam session starts

- First development session. The jam window opened at 07:00; the morning went into the last revision of the plan
  (`JAM_PLAN.md` v2: the concept was cut down to a single boss fight).
- The plan in two lines: one arena, one boss, the Publisher. At health thresholds he steals a mechanic the player
  chooses to give away and uses it against the player, while the game around degrades down to a bean with a ball
  shot; at zero everything returns for the final blow.
- Today: the whole fight on beans (blocks A, B, C), three switchable cameras, a packaged Shipping build at 23:00.
- Tools in use: Claude Code as the only coding agent, the UE 5.8 editor through MCP, `build_and_test.sh` for the
  build and the `Jam.` tests. Concept images go through fal in the background.

### 13:40 Tickets, concept round 1, butler

- Blocks A, B and C are 15 tickets in `Tickets/`, each with its test seam, plus the bot and the K1 build; blocks D
  to G are one-line stubs. Architecture decision: the fight is a pure fixed-step simulation on plain data, fighters
  are driven by intents, so the hero, the boss and a bot share the mechanic code and a whole fight runs in a test.
- Concept round 1 through fal (GPT Image 2.6, written by Claude Code as `Tools/Art/fal_images.py`): 9 images in
  parallel, about 17 seconds each, booked as 0.90 USD of the 5 USD cap. Two heroes, two bosses, a size lineup, two
  arena moods, the katana and the shuriken as separate props. Sent to the author; the choice is Q2 in `QUESTIONS.md`.
- What went wrong: the name NEON RONIN is taken by an unreleased Steam game (Q1). butler 15.32.0 is installed
  outside the repository; the push waits for the itch page address.

### 14:10 Block A: the bean, three cameras, the ball

- What works: a round arena built in code, the hero bean moves and aims, three cameras on F1 (top-down 3/4), F2 (over
  the shoulder) and F3 (first person), the weak ball shot, the boss dummy with a health bar. The simulation never
  learns which camera is on: the camera only changes how the keys and the mouse become a move direction and an aim.
  33 `Jam.` tests, each written before its code; the first build of the block passed them all.
- New tool made by the agent for itself: a demo run (`-JamDemo`). A bot plays the hero through the same intent as a
  person, the camera goes through the three modes, every frame is saved and ffmpeg joins them into a clip
  (`Tools/Build/demo_clip.sh`). The author is away, so this is how the agent looks at its own game. First clip:
  `Clips/block_a.mp4`.
- What went wrong: the first look showed the hero bean covering the crosshair in the shoulder camera, a HUD line over
  the boss bar, motion blur smearing the ball and a plain floor on which running reads as standing. Fixed from the
  frames in one more round: camera offset, floor grid, blur off, one exposure for all cameras.

### 14:45 Block B: katana, shuriken, dash, and a Publisher who fights back

- What works: every mechanic has one owner in a ledger, and the hero and the boss run the same code for it, so a steal
  is one changed line of data. Katana with a three-strike series and a finisher, shuriken where every third explodes,
  dash with a moment of invulnerability. The Publisher chases, throws a fan of five memos and slams a circle that
  warns first. A button whose mechanic is gone shoots the weak ball. Win and loss titles, R restarts, F5 is a debug
  steal until block C makes the real one. 68 `Jam.` tests, written before the code; the first build passed them all.
- The agent's eyes: the bot now fights with everything the hero owns and dodges, the demo run lets the Publisher take
  a mechanic every 8 seconds in its second half, and a contact sheet of one frame per second (ffmpeg) is how a
  48 second clip is checked in one look. Clip: `Clips/block_b.mp4`.
- What went wrong: nothing broke, but the bot kills the Publisher in about 12 seconds and the fight has to last 3 to
  4 minutes. The numbers live in one ini section, the balance pass comes with block C and is measured by the bot.
  In first person a melee fight means the boss bean fills the whole screen: one more point for the top-down camera.

### 15:20 Block C: the Publisher has notes

- What works: the whole loop of the game on beans. At 75 and 50 percent of his health the Publisher stops the fight
  and puts two mechanics on a card, the player gives one away with A or D (no answer in 3 seconds: he takes the left
  one), at 25 percent he cuts the rest and the hero is a bean with a ball. What he took he uses: a slow katana with
  a readable sweep, shuriken, a dash out of the hero's strikes. A dead hero retries from the last cut with full
  health. At zero the Publisher is stunned, everything returns and the next hit ends it: "NEON RONIN: DIRECTOR'S
  CUT". 94 `Jam.` tests, written before the code; the first build passed them all.
- The agent's eyes: the demo run now plays real steals instead of planned ones, retries after a loss and takes a
  camera by number, so one script records a clip per camera. In the 60 second clip the bot loses to the bean stage
  once, retries from the last cut and wins: every screen of the loop is in it. Clip: `Clips/block_c.mp4`.
- What went wrong: the ticket asked for slow motion during the card; the fight freezes instead, because A and D are
  both the move and the answer keys and a frozen, cleared stage makes the retry point a plain copy. One test was
  wrong on paper before it ran: a hit dealt outside a simulation step is invisible to the step that settles the
  outcome. The fight is still far too short (under a minute with a death in it, the target is 3 to 4): the bot
  measures the balance pass next.

### 15:30 The bot plays whole fights: the balance has a test

- What works: a fight from the first step to a title runs without a screen, 60 of them in one test. The bot
  answers the steal card by a policy, so the tests see all six orders in which the first two mechanics can go.
  New tests on the real numbers: the same seed gives the same fight, every fight ends, every order can be won, the
  median won fight lasts 2 to 5 minutes. 101 `Jam.` tests.
- The numbers: the boss's health went from 1000 to 4000, the median won fight from 35 to 168 seconds.

| Boss health | Bot | Wins | Median of a won fight | Closest win |
|---|---|---|---|---|
| 1000 | runs in whenever the boss rests | 60 of 60 | 35 s (31 to 46) | 44 health left |
| 4000 | runs in whenever the boss rests | 34 of 60 | 168 s (126 to 175) | 74 health left |
| 4000 | runs in only with the dash | 60 of 60 | 168 s (126 to 182) | 74 health left |

- What went wrong: with four times the health the bot lost every fight in which it gave the Dash away early, 26 of
  26. It kept running in with the katana, and next to the Publisher a volley of memos cannot be walked around. The
  bot now keeps its distance once the dash is gone, and that shows the opposite problem: a hero who circles at
  range is almost never hit. The Publisher needs an answer to kiting before the fight is a fight.

### 15:55 The Publisher leads his volley: winning now costs health

- What works: the memo volley is thrown where the hero will be, so running in one circle no longer dodges it.
  The answer is to change direction when he throws, or to dash through. The bot learned the same, plus two things
  a person does without thinking: it keeps room between itself and the wall, and it counts an explosive shuriken
  where it ends, not only where it flies. 107 `Jam.` tests.

| Volley | Bot | Wins | Median of a won fight | Closest win |
|---|---|---|---|---|
| straight | circles one way | 60 of 60 | 168 s (126 to 182) | 74 health left |
| led | turns its circle | 0 of 60 | none | hero dead, boss at 340 to 1460 |
| led, no led shuriken | also runs along the wall | 52 of 60 | 164 s (126 to 175) | 8 health left |
| led, no led shuriken | keeps off the wall | 60 of 60 | 167 s (126 to 176) | 5 health left |

- What went wrong: the first version lost every fight, and not to the volley. The stolen shuriken shares the aim
  of the Publisher, so one shuriken per volley was led by accident. And the bot backed into the wall, where every
  third stolen shuriken explodes right behind it. The numbers of the fight were not touched: the two fixes are in
  the boss's brain and in how the bot moves. Finding for the look of the game: an explosive shuriken in the hands
  of the Publisher must be visible as one, or the wall punishes a player who cannot know why.

### 16:05 K1: the Shipping build exists, one clip per camera

- What works: a Shipping package of the whole fight, built from commit `136fe26` (tag `k1`) and checked by a run of
  the packaged exe with the bot. It contains the bean with the ball, the katana, the shuriken and the dash, the
  Publisher with his volley and slam, three steals with the choice card, the stolen mechanics in his hands, the
  stun and the last blow, both titles and the retry from the last steal. Three clips of the same fight, one per
  camera: `Clips/k1_cam0_topdown.mp4`, `Clips/k1_cam1_shoulder.mp4`, `Clips/k1_cam2_firstperson.mp4`.
- What is known to be broken or missing: the debug lines (F5, F6, the camera keys) are on screen in Shipping; an
  explosive shuriken looks like a plain one in the hands of the Publisher; no sound, no music, no art beyond
  capsules; the fight length is measured with the bot only, no person has played it. The build is not on itch:
  the page address is not known yet, and the zip (303 MB) is too big for the file channel.
- Question to the author: which camera? Top-down is the default if there is no answer by Friday 07:00.

## Fri 13 Nov

### 23:59 The camera is chosen, and a day is lost

- What works: the author picked camera 0, top-down, from the three K1 clips. The hand-off for the next session is
  in `HANDOFF.md`.
- What went wrong: the session stood still from Thursday 16:05 to Friday night. Nothing of Friday's plan (blocks D
  and E) is started, the test build of 18:00 was not made, and the K1 build never reached the author: no itch
  address yet, and the zip is too big for the file channel. The next session opens with the question of what to
  cut.

## Sat 14 Nov

### 01:00 T-018: one camera, and the road to itch

- What works: only the top-down camera is left. F1 to F3 and the mouse look are gone, the demo plays top-down, and the
  debug keys F5 and F6 with their hint are compiled out of Shipping. Clip `Clips/t018_topdown.mp4`. 107 `Jam.` tests
  green. Tool: Claude Code.
- What went wrong: the first `butler push` (butler v15.32.0, now installed) to `pavyny/neon-ronin` was refused by itch
  because the account email is not verified. That one is on the author (`QUESTIONS.md`, Q4b). Priority is now the
  degradation first: T-023 and T-024 before Parry and Ultimate.

### 01:20 T-023: the road down to the bean, on stand-ins

- What works: five cosmetic layers, cut by the Publisher in `[JamLayers]` order: effects (glow, blade sweep, dash
  trail) at cut 1, materials (the hero and the arena go to the engine grid) at cut 2, animation (T-pose), model (the
  ronin of basic shapes becomes the bean) and HUD bars at cut 3. The stun gives everything back; the Publisher keeps
  his polish all the way. Pure rule `JamLayerRules`, 5 new tests (red first), 112 green. Shipping package
  `Saved/Packaged/Windows/JamGame.exe`. Clip `Clips/t023_road_to_the_bean.mp4` (200 s of a bot fight). Tool: Claude Code.
- What went wrong: `AActor` already has a member `Layers` (a compile error on the first build). The 200 s clip ends
  before the stun, so the way back is not on it. Itch still refuses the push (unverified email).

### 01:35 T-024: the band leaves one by one

- What works: four stub stems (drums, bass, pad, lead) and a metronome loop, synthesised by a script made in the jam
  (`Tools/Audio/make_stub_stems.py`), imported by a commandlet. They loop together; cut 1 fades the lead, cut 2 the
  pad, the final cut bass and drums, and the metronome is left. The stun brings the band back. 3 new tests (red first),
  115 green. Tool: Claude Code.
- What went wrong: the first stems clicked at the loop point (a low-pass that was not circular). The bot clips are
  frames without sound, so the evidence is the engine list of active sounds, `Clips/t024_active_sounds.png`.

### 01:40 T-019: Parry, the fourth mechanic

- What works: Q opens a 0.25 s window; a projectile that reaches the hero in it flies back at the Publisher, three
  times as hard. Ring and flash on stand-ins, a HUD row, 6 new tests (red first), 121 green. The bot parries lone
  projectiles and dashes through fans: 60 of 60 wins in all 12 steal orders, closest win now at 36 health (was 5).
  Tool: Claude Code.
- What went wrong: a bot that parried everything won 16 of 60, because one window does not cover a five-memo fan. Six
  older tests assumed three mechanics; they count from the mechanic count now. The Publisher does not use a stolen
  parry yet (cut line item 3, the author decides).

### 01:50 T-020: the Ultimate, the fifth mechanic

- What works: damage dealt fills a meter; full, E winds up in slow motion (0.3) and slashes along a 16 m line. Ring,
  slash line, meter in the HUD. The bot uses it. 6 new tests (5 red first), 127 green, bot 60 of 60 in all 20 steal
  orders. Tool: Claude Code.
- What went wrong: the first numbers (300 damage) made bot fights too short (median 117 s, floor 120); now 150 damage
  and a slower meter, median 121 s: T-021 retunes the cuts. The steal-order test needed its own seeds to reach all 20
  orders. The first clip recording was cut off by the session; recorded again.

### 02:05 T-021: five mechanics, four cuts

- What works: cards with a pair at 80, 60 and 40 percent, the FINAL NOTE at 20 takes the last two. One cosmetic layer
  and one music stem per cut: effects, grid, T-pose, then the bean with the metronome. 127 green, bot 60 of 60.
  Tool: Claude Code.
- What went wrong: changing the round numbers of the test balance broke tests that pin 750/500/250; the test numbers
  went back and the fourth threshold got its own.

### 02:15 T-022: the Ultimate lands the final blow

- What works: at zero the Publisher is stunned, everything returns (mechanics, layers, the band), the meter is full and
  only the slash ends the fight; a katana hit on him only fills the meter. 128 green, bot 60 of 60. Block D is done.
  Clip of a whole fight `Clips/t022_road_down_and_back.mp4`. Tool: Claude Code.
- What went wrong: nothing new; the cut-line question (does the Publisher use a stolen Parry or Ultimate) is still
  open, so he only takes them.

### 02:25 Block D on itch, hand-off

- What works: the author verified the itch account; the block D Shipping build is pushed to `pavyny/neon-ronin`
  (channel `windows`, butler). The push script is in the repo now (`Tools/Build/itch_push.sh`). `HANDOFF.md` holds the
  state and the prompt of the next, autonomous session. Tool: Claude Code, butler.
- What went wrong: a bare `butler status` without the key waits for a login and hung for two minutes.

### 02:35 T-029: readable blows (autonomous session 3)

- What works: hits leave a spark sized by the damage, the camera shakes on the slam, the Ultimate, explosions and
  the hero hurt; the Publisher flashes by the size of the blow, not white on every ball; the parry ring closes with
  its window; his explosive shuriken is yellow, pulses and drags its blast circle. 6 new tests (red first), 134
  green. Clip `Clips/t029_readability.mp4`. Tool: Claude Code.
- What went wrong: nothing; the 30 s clip has no steal of the shuriken, so the blast ring waits for the
  whole-fight clip.

### 02:40 T-028 part: sound effects on stubs

- What works: thirteen sounds, one per thing the player must hear (katana, throw, explosion, slam, dash, parry,
  Ultimate, boss hit, hero hurt, the card, the steal, everything back, the final blow), picked from the step events
  once per frame. Synthesised stubs under the names Kirill delivers to. 5 new tests (red first), 139 green.
  Evidence `Clips/t028_active_sounds.png` (`au.Debug.Sounds`). Tool: Claude Code, Python (numpy).
- What went wrong: the import commandlet loaded the stub build and logged empty asset paths; harmless, the import
  went through.

### 02:50 T-027: the neon arena on stand-ins

- What works: an unlit neon material (made by an editor Python script), glowing posts, a neon band at the edge,
  floor lines that pulse on the 120 BPM beat, glowing effects and shots, bloom that goes with the effects layer.
  3 new tests (red first), 142 green. Clip `Clips/t027_neon_arena.mp4`. Tool: Claude Code, UE Python.
- What went wrong: the beat test caught a rounding epsilon that dimmed every beat; the posts at glow 4 washed out to
  pastel in the clip and were toned down after it.

### 02:55 T-031: balance off the floor

- What works: one line in `[JamBalance]`, the Publisher's health 4000 to 4800: the bot median went from 122 to
  142 s (left 153, right 151, mixed 141), 60 of 60 won, all 20 steal orders won. New test keeps every policy 15 s
  above the floor (red first on the old numbers). 143 green. Clip `Clips/t031_boss_health_4800.mp4`. Tool: Claude Code.
- What went wrong: nothing; a person's fight is still not measured.

### 03:10 Session 3 end: Shipping on itch, whole-fight clip, hand-off

- What works: Shipping build packaged and smoke-run (still running after 30 s), pushed to itch as `session-3`
  (build #2380360). Whole-fight clip `Clips/session3_whole_fight.mp4` shows the blast ring under his explosive
  shuriken. Report `Reports/session3_report.md`, `HANDOFF.md` for the next session. Tool: Claude Code, butler.
- What went wrong: nothing blocking; all four tickets were done by 02:51, so the end block started early.

### 11:45 Session 4: the author plays three fights, answers the open questions

- The author's playtest of the session-3 build: the aim felt sluggish, Parry and the Ultimate were not discoverable,
  melee showed nothing after the first cut. Fixes chosen in chat (T-034 to T-037). Answers: the name RONIN: FINAL CUT,
  keep everything on the cut line (the Publisher uses his stolen moves, an intro, DLSS), 13 sound names approved,
  4800 health kept, team name Pavyny. Tool: Claude Code.
- New brief for the Publisher: a heavy fat-cat boss. Concept round 2 through fal (4 images, 0.40 USD), the author
  picked C3 without the belly plate (`r2_boss_c3b`), with a heavy profile of numbers. Tool: fal, GPT Image 2.6.

### 12:13 T-034: the hero turns to the mouse at once

- What works: the hero model turns to the cursor on every rendered frame, from where it is drawn, through this frame's
  camera; a neon hardware pointer that Windows draws, so it never trails the mouse. 2 new tests (red first), 145
  green. Tool: Claude Code, Python (Pillow) for the pointer.
- What went wrong: no clip can show it: the bot demo has no mouse and a captured frame has no hardware cursor. The demo
  log shows both pointer images loaded.

### 12:19 T-035: melee is always seen, Parry is a shield

- What works: after the effects cut the hero's blade is a flat grey stand-in instead of nothing; the parry window is a
  standing cage of glowing panels round the hero instead of a flat disc that read as an attack. 1 new test (red
  first), 146 green. Clip `Clips/t035_bare_swing_shield.mp4`. Tool: Claude Code.
- What went wrong: nothing; in the clip the bot dies once after losing Parry and retries from the cut.

### 12:25 T-036: the game teaches Parry

- What works: when an enemy shot is 0.6 s from landing and Q would open the window, "Q PARRY: SEND IT BACK" shows over
  the hero and the shot gets a cyan ring, until the player has deflected three shots in the session. 4 new tests (red
  first), 150 green. Clip `Clips/t036_parry_prompt.mp4`. Tool: Claude Code.
- What went wrong: nothing; the bot parries at once, so in the clip the prompt is short. A person sees it longer.

### 12:38 T-037: a full Ultimate is called out

- What works: a magenta meter bar under the health bar; once it is full a pulsing "E ULTIMATE READY" banner, a ring at
  the hero's feet, a chime (stub cue 14, `sfx_ultimate_ready_01`) and, until he has used it once, a call over his
  head. 3 new tests (red first), 153 green. Clip `Clips/t037_ultimate_ready.mp4`. Tool: Claude Code, Python (numpy).
- What went wrong: the first clip showed no banner at all: the bot presses E on the very step the meter fills. A demo
  switch that keeps its hands off E (`-JamDemoHoldUltimate`) made the clip; the rule was right.

### 12:42 Shipping build of the four playtest fixes

- What works: `Pavyny_NeonRonin_Session4.zip` packaged and smoke-run (still running after 30 s); the neon pointer is in
  the pak manifest. Tool: Claude Code, UAT.
- What went wrong: the push to itch was held for the author's explicit go-ahead.

### 12:46 T-041: the heavy Publisher

- What works: the numbers of a slow, big, hard-hitting boss (speed 330 to 260, radius 70 to 90, wind-ups longer,
  slam 24 to 34). New test keeps him heavy against the hero (red first). Bot: 60 of 60, median 151 s, every steal
  order won. 154 green. Clip `Clips/t041_heavy_publisher.mp4`. Tool: Claude Code.

### 13:22 The game's sound made with ElevenLabs (T-028)

- What works: on the author's word, the sound designer's task done as an example and fallback: 14 sound effects and a
  bare prototype loop from our own prompts; one 40 s synthwave track at 120 BPM split into stems and cut to four 8-bar
  layers in step (the pad and the lead came mixed in one stem and are split by our script). Levelled by role and
  imported over the stubs under Kirill's names. Brief for Kirill written; the build pushed to itch as `session-4a`.
  All of it on the concept page, Sounds tab. Tool: ElevenLabs (Creator plan), Claude Code, ffmpeg, numpy.
- What went wrong: the stem split returned six stems, three of them silent (piano, guitar, vocals). Nobody has heard
  the mix inside the game yet.

### 13:36 Session 4 end: the sound on itch, boss C2 round 3, hand-off

- What works: Shipping build with the ElevenLabs sound smoke-run and pushed to itch as `session-4b` (build #2383825).
  The author liked the sounds and picked boss C2; round 3 of concepts (C2 without the belt, three satirical takes
  with dollar eyes, a coin crown, a rejection stamp) through fal, 0.40 USD. `HANDOFF.md` and
  `Reports/session4_report.md` for the next session. Tool: Claude Code, fal, butler.
- What went wrong: nothing blocking; the itch push waits for the author's explicit word each time.
- What went wrong: the bot's closest win went from 23 to 47 health: it finds the slow boss easier. A person's hands
  decide whether he needs more health.

### 14:23 The Publisher bleeds paper dollars (T-042)

- What works: session 5 opened with the author at the PC; questions answered in one round (bleed look, the boss's
  stolen Parry and Ultimate, a Meshy cap of 400 credits, Kirill's files not in). Each hit on the Publisher sprays green
  bills by the damage; they flutter down, lie on the floor and fade, 60 on the final blow. Tests first. Tool: Claude
  Code.
- What went wrong: the first try drew all bills as one instanced mesh and they came out grey (the neon material is
  most likely not flagged for instanced meshes); switched to a pool of plain cards.

### 14:30 Playtest numbers: a shorter fight (T-043)

- What works: the author played and sent five findings. First the numbers: 4000 health, the ball-only stretch after
  the FINAL NOTE cut from 960 to 120 health, 7 s on the steal card. Bot median 151 to 92 s, 60 of 60 won. Tool:
  Claude Code.
- What went wrong: the bot length floor (120 s) failed by design; lowered to 60 s on the author's word.

### 14:35 Parry on the strike (T-044)

- What works: Q is gone; LMB parries when an enemy shot would land within the parry window, otherwise it swings.
  The bot parries the same way. Tests first. Tool: Claude Code.
- What went wrong: the bot's closest win dropped from 87 to 16 health: the strike-parry is a little harder for it.

### 14:39 An arrow to the Publisher off screen (T-045)

- What works: a red wedge at the screen edge points at the Publisher whenever the camera has lost him. Test first.
  Tool: Claude Code.
- What went wrong: nothing; the canvas triangle needed the `RenderCore` module.

### 14:46 The Ultimate shows where and when (T-046)

- What works: while the Ultimate is ready a magenta lane on the floor follows the aim (bright when the Publisher is
  in it); after E it locks and fills to its end exactly when the cut lands. Test first. Tool: Claude Code.
- What went wrong: from the top-down camera most of the 16 m lane runs off the screen; the near part reads.

### 14:55 Build session-5a on itch

- What works: Shipping packaged in 3 min, smoke-run (still running after 20 s), pushed as `session-5a` (build
  #2384201) on the author's word. Tool: Claude Code, butler.
- What went wrong: nothing.

### 15:17 The boss arrow keeps clear of his bar (T-047)

- What works: the author played `session-5a` and liked it; three changes asked. First: the edge arrow now stops
  16 px under the Publisher's health numbers instead of touching the bar. Test first. Tool: Claude Code.
- What went wrong: nothing.

### 15:20 A retry keeps the health of the cut (T-048)

- What works: the checkpoint stores the hero's health when the steal resolves; a retry brings it back, or 50% if it
  was 20% or less. Test first; the old retry test lost its "whole again" check. Tool: Claude Code.
- What went wrong: no clip: the bot never dies, so it never retries.

### 15:36 The Publisher drops a cash-stack heal (T-049)

- What works: every 25 s the next hit on the Publisher knocks a glowing cash stack out towards the hero; touching it
  heals 25 with green rings and a cash register "ka-ching" (ElevenLabs, cue 15 on Kirill's list). The bot goes for
  it; 60 of 60 still won, median 92 s. Tests first. Tool: Claude Code, ElevenLabs.
- What went wrong: the first clip showed no drop (the bot died at 24 s and the retry restarts the timer); the clip was
  recorded with an 8 s interval. The first stack was too small to read from the camera and was made 1.6 times bigger.

### 15:41 Build session-5b on itch

- What works: Shipping with T-047 to T-049 packaged, smoke-run (still running after 20 s), pushed as `session-5b`
  (build #2384521) on the author's word. Tool: Claude Code, butler.
- What went wrong: nothing.

### 16:04 The slam grows with each cut (T-050)

- What works: the Publisher's slam is one circle before the first cut, three after it (around him, on the hero,
  across the way back), five from the second cut on; they land 0.2 s apart from him outwards. A seeded pure rule
  places them, never over a cash stack, and drops a cutter if it would leave nowhere to go. A sweep of 14808
  patterns over the whole arena proves a spot the hero reaches walking after 0.3 s. The bot now walks to that spot.
  Bot 60 of 60, median 92 to 83 s. Tests first. Tool: Claude Code.
- What went wrong: the old bot rule only left the circle it stood in; at the wall it walked into the next one
  (the new bot test was red on it, 100 to 32 health).

### 16:15 Build session-5c on itch, a progress report

- What works: Shipping with T-050 packaged, smoke-run (still running after 20 s), pushed as `session-5c` (build
  #2384756) on the author's word. A mid-session report in `Reports/session5_progress.md`. Tool: Claude Code, butler.
- What went wrong: the first packaging attempt failed at once: another AutomationTool was running on the PC; the
  build waited 70 s for it and went through.

### 16:46 The Katana is untouchable (T-051)

- What works: the author played `session-5c` and liked it; four changes asked. First: a choice card never offers the
  Katana, so it is always the last one left and only the FINAL NOTE takes it. Test first. Bot 60 of 60, the 12 steal
  orders (20 before) all won, median 83 to 78 s; the author moved the bot floor from 60 to 50 s instead of adding
  boss health. Tool: Claude Code.
- What went wrong: nothing; two old tests still counted five left options on the card and were updated with the rule.

### 16:50 Build session-5d

- What works: Shipping with T-051 packaged (`Pavyny_NeonRonin_Session5d.zip`), smoke-run (still running after 30 s).
  Not pushed: waits for the author's word. Tool: Claude Code.
- What went wrong: nothing.

### 17:03 The five-circle slam spreads out (T-052)

- What works: from the second cut the slam is two circles at the Publisher's flanks, one on the hero and two wide
  behind him, with at least 200 cm of floor between any two: pockets to stand in. Each circle searches a ring around
  its anchor for room. Tests first; a sweep of 4936 five-circle patterns finds no gap under 200 cm, every pattern
  still leaves a spot to walk to. Bot 60 of 60, median 79 s. Tool: Claude Code.
- What went wrong: the first placement pushed and clamped circles and dropped most of them near the wall (37% of the
  fives stayed whole); a ring search with 4 rings got 48%, with 8 rings all of them.

### 17:05 Build session-5e

- What works: Shipping with T-052 packaged (`Pavyny_NeonRonin_Session5e.zip`), smoke-run (still running after 30 s).
  Not pushed: waits for the author's word. Tool: Claude Code.
- What went wrong: nothing.

### 17:10 Health bars with a delayed chunk and a heal preview (T-053)

- What works: on both health bars a hit leaves the lost part white; it grows while hits keep coming and drains 0.6 s
  after the last one. A heal shows light green at once and the bar fills up to it. A pure rule for the shown values
  (`JamBarRules`), numbers in `[JamBar]`, the simulation untouched. Tests first. Tool: Claude Code.
- What went wrong: nothing.

### 17:13 Build session-5f

- What works: Shipping with T-053 packaged (`Pavyny_NeonRonin_Session5f.zip`), smoke-run (still running after 30 s).
  Not pushed: waits for the author's word. Tool: Claude Code.
- What went wrong: nothing.

### 17:18 The dollars spill out of him (T-054)

- What works: paper dollars leave the edge of the Publisher's body and land outside it on an arc, all round him; the
  cash stack flies from his edge too. Tests first. Tool: Claude Code.
- What went wrong: short throws from his centre used to land inside his body. The new test also found a cash stack
  landing in him against the wall; it now goes towards the middle of the arena there.

### 17:25 Build session-5g

- What works: Shipping with T-054 packaged (`Pavyny_NeonRonin_Session5g.zip`), smoke-run (still running after 30 s).
  Not pushed: waits for the author's word. Tool: Claude Code.
- What went wrong: the T-040 window title went into the ini while it was packaging; the build may already carry it.

### 17:36 The game is called RONIN: FINAL CUT (T-040)

- What works: the win title and the window title say RONIN: FINAL CUT (NEON RONIN is taken on Steam). Test first.
  Tool: Claude Code.
- What went wrong: the clip was meant to end on the win title, but in that run the bot died at the very end and
  retried; the title is checked by the test.

### 18:31 What he stole hits at full strength (T-055)

- What works: the author lifted the Saturday freeze (features stop Sun 12:00, a Shipping build on itch at every
  milestone) and asked for a more aggressive Publisher. Stolen damage 0.9 (was 0.6), cooldowns x1 (was x2), shuriken
  speed 0.8 (was 0.5), his pause 0.7 s (was 1.1). Bot 60 of 60, median 81 s. Tool: Claude Code.
- What went wrong: nothing; the bot barely feels the change, a person's run is the real test.

### 18:54 He uses the Parry and the Ultimate he stole (T-038)

- What works: his stolen Parry is a shield stance (0.3 s rising out of the floor, 1.2 s that sends hero shots back,
  4 s cooldown); his stolen Ultimate fills on a 14 s clock, winds up 0.9 s with his red lane, the line locked, 35
  damage. Only the hero's wind-up slows the world now. Bot 60 of 60, median 108 s. Tests first. Tool: Claude Code.
- What went wrong: the clip script padded short runs with frames of older runs, so the T-055 clip ends on old
  footage (a bot death that was not this build). Fixed: a fresh folder per run, the run stops on its frame count.

### 19:09 A signature chain per stolen mechanic (T-056)

- What works: from the first cut the Publisher strings attacks together after a 0.35 s tell (a red ring): slam then
  a fan into the pocket; with the Dash, dash in and slam; Shuriken, fan then a led burst; Parry, the shield then a fan
  as it drops; Ultimate, a fan then the line; Katana, a fan then a rush. Never the same chain twice in a row, 5 s
  between chains. Bot 60 of 60, median 97 s, closest win at 26 health. Tests first. Tool: Claude Code.
- What went wrong: the pocket fan first aimed at "the spot reachable now", which a hero standing still never has; it
  now aims at the pocket the pattern offered when it went down.

### 19:14 An early parry click waits for the shot (T-057)

- What works: a strike up to 0.3 s before a shot reaches the parry window is held (no swing) and parries the shot
  when it comes in. Also fixed an off-by-one: a parry opened on time could still let a shot through on its last step.
  Tests first. Tool: Claude Code.
- What went wrong: the new test stayed red after the hold was in; tracing the steps showed the off-by-one in the old
  look-ahead, probably part of the author's "I clicked and it hit anyway".

### 19:26 The Ultimate turns toward the cursor during its wind-up (T-058)

- What works: after E the hero's line keeps turning toward the cursor at 120 degrees per second (about 40 degrees
  over the wind-up); the lane follows it. The Publisher's line stays locked. Tests first. Tool: Claude Code.
- What went wrong: the clip saved 20 frames (the game window was covered); recorded later.

### 19:30 Build session-5i on itch: the fight block

- What works: Shipping with T-055, T-038, T-056, T-057, T-058 packaged (`Pavyny_RoninFinalCut_Session5i.zip`),
  smoke-run (still running after 30 s), pushed as `session-5i` (build #2386306) on the author's word in the session
  brief. 205 `Jam.` tests, bot 60 of 60, median 99 s, closest win at 26 health. Tool: Claude Code, butler.
- What went wrong: the T-058 clip saved only 20 to 23 frames twice: the game window stops rendering while covered.

### 20:27 Feedback on session-5i: the parry and his shurikens (T-059, T-060)

- What works: a parry that sends a shot back is ready again at once, so spamming LMB into a burst sends back every
  shot (it was every other one); an empty parry keeps its 0.8 s cooldown. The Publisher's stolen shurikens never
  explode and the parry catches them 30 cm past the body; the hero's every third still explodes (the author's pick).
  Bot 60 of 60, medians 102 / 97 / 101 s, closest win 26 / 44 / 60 health: a little easier, not too easy by the bot.
  Clip `Clips/t059_t060_spam_parry.mp4` with a new demo switch `-JamDemoSpamStrike`. 209 tests. Tool: Claude Code.
- What went wrong: "never explodes" as a zero in `[JamBalance]` failed the validator (every zero reads as a missing
  line); the rule moved into the throw. The burst test first started its last memo outside the test arena.
- Build: Shipping `Pavyny_RoninFinalCut_Session5j.zip`, smoke-run (still running after 30 s), pushed as `session-5j`
  (build #2386825, processing at push).

### 20:54 The 3D models, first round, for the author's review

- What works: four new sheets on fal (the Publisher in a wide A-pose so the rig keeps his arms off his belly, the cash
  stack, dollar and memo card textures, 0.40 USD). The hero through the Meshy API (Meshy 7.1, T-pose, 20.6k tris),
  rigged on UE5 Mannequin bone names with 7 library clips; the Publisher's rig and 7 heavy clips the same way: 82 of
  the 250-credit cap. All of it on the art page's new Models tab with turnable, animated views. Nothing is imported
  yet. Tool: Claude Code, `Tools/Art/meshy_models.py` (credit ledger, cap checked against the balance).
- What went wrong: the first round went to fal for 3D as the brief said; the author then moved 3D and rigging to the
  Meshy API directly. The fal round was stopped with 4 of 5 done (2.78 USD: the Publisher, katana, shuriken, cash
  stack; the author keeps them), the hero never reached fal. Artifacts do not serve .glb files, so the page carries
  the models as base64 text; gltf-transform failed on this machine (libvips), so a small Python shrinks the textures.

### 21:49 The Publisher's rig rebuilt and exported

- What works: the Meshy auto-rig tore the jacket on every arm and head move (shoulder joint 25 cm inside the chest,
  jacket front weighted to the clavicles and the head). Rebuilt in headless Blender 5.2 by our scripts
  (`Tools/Art/publisher_rig`): joints refitted under the same bone names, weights painted from scratch from rest
  position and texture colour (4 influences, normalised), a `root` bone, a belly helper with a damped follow-through
  and three jacket flap bones. The seven clips reworked: on the origin and on the ground, arms kept out of the torso,
  travel on `root`, plus a T-pose. Before and after on the art page (Models tab), approved by the author, exported as
  `SK_Publisher.fbx` and one FBX per clip to `Art/Models/boss_fixed` with `README.md` as the import notes. Read back
  into Blender: 27 bones, every bone within 0.001 cm of the source. Tool: Claude Code.
- What went wrong: the author found a lapel sticking out in the idle: shaded creases of the collar passed the skin
  colour test and were painted as head; the head set is now only what joins the skull, and stacked layers (lapel,
  jacket edge, tie) share weights over space. The first FBX export scaled the mesh twice (260 m tall), caught by the
  read-back check. Not imported into Unreal and not checked there: the Unreal MCP did not connect in this session.

### 22:05 The Publisher's rig, round 3: a tie of its own and the slam hand

- What works: two findings of the author fixed in `Tools/Art/publisher_rig`. The tie was skinned to the chest and the
  belly, so it bent when the torso twisted in the idle: it now has two bones (`tie_01`, `tie_02`, 29 bones in all)
  and a baked motion: it hangs from the knot, lies on the belly, follows gravity sideways, lifts off when he bends
  forward and lags a little. In `Charged_Ground_Slam` the raised hand was cocked back with the palm up: the raised
  arm now turns its palm forward (the turn shared by arm, forearm and hand) and the wrist bend is cut to 12 degrees.
  All FBX files exported again and read back (29 bones, every bone within 0.001 cm), art page and `README.md`
  updated. Tool: Claude Code.
- What went wrong: the skeleton changed after the first export, so anything imported from it has to be imported
  again. Still not checked in Unreal: the Unreal MCP did not connect.

### 22:21 The Publisher's rig, round 4: the raised hand of the slam

- What works: in the wind-up of `Charged_Ground_Slam` the right hand still bent outward at the cuff. The fingers of
  a raised hand now continue the line of the sleeve and lean a little upward (`HAND_UP` in
  `Tools/Art/publisher_rig/rework_clips.py`). Only that clip changed; all FBX files exported again and read back
  (29 bones, every bone within 0.001 cm), art page and `README.md` updated. Tool: Claude Code.
- What went wrong: the previous fix limited the wrist bend against the forearm bone, but the wide sleeve reads
  steeper than the bones under it, so the hand looked bent while the numbers said 12 degrees. Judged by renders
  from the author's view this time. Still not checked in Unreal.

### 22:25 The Publisher's rig, round 5: the tie cut loose from the shirt

- What works: when the tie swung away from the belly it pulled black spikes of the shirt with it. `build_rig.py`
  now copies the tie faces as a free two-sided strip on the tie bones and leaves the faces they came from on the
  body as plain black shirt (`split_tie`). The mesh grew from 12 784 to 13 086 vertices; skeleton and clips are the
  same. All FBX files exported again and read back, art page and `README.md` updated. Tool: Claude Code.
- What went wrong: Meshy models the tie into the shirt as one surface, so giving it bones of its own in round 3
  stretched the faces on the border. Still not checked in Unreal.

### 22:31 The Publisher's rig, round 6: a straight raised arm

- What works: the author still saw the raised hand of the slam as curved, this time from behind. The raised arm now
  gives up 85% of its elbow bend and the hand follows the shoulder-wrist line (`ARM_STRAIGHT`, `HAND_UP` in
  `rework_clips.py`), so it is one straight line from every side. Only `Charged_Ground_Slam` changed; FBX files
  exported again and read back, art page and `README.md` updated. Tool: Claude Code.
- What went wrong: round 4 leaned the hand toward vertical to match the sleeve as seen from the front; with the
  elbow bent the other way that made an S-curve from the back. Checked from six views this time, not one.

### 22:55 The Publisher's rig, round 7: the raised arm goes up, not out

- What works: the author still saw the raised arm of the slam as curved. The arm leaned about 43 degrees out
  while the fat shoulder stays upright with the torso, so the sleeve read as bent in the middle. A raised arm now
  keeps only 30% of its lean beyond 12 degrees from straight up (`ARM_LEAN`, `ARM_LEAN_KEEP` in
  `rework_clips.py`) and the hand leans a little more toward vertical (`HAND_UP` 0.3). Only `Charged_Ground_Slam`
  changed; FBX files exported again and read back, art page and `README.md` updated. Tool: Claude Code.
- What went wrong: round 6 straightened the elbow but left the whole arm leaning against an upright shoulder. The
  headings of rounds 4 to 6 were written with guessed times that ran ahead of the clock; they now carry the
  commit times. Still not checked in Unreal.

## Sun 15 Nov

### 00:30 The Publisher's rig, round 8: neck border and a gold knot

- What works: skin spilled onto the collar under the chin and black onto the jowls, and the top of the tie knot was
  dark with a dot of skin. `build_rig.py` now cuts the skin-to-collar border into the mesh along a smooth line over
  the surface (`neck_border`: plain black collar on one side, repainted skin on the other) and paints the top of the
  knot gold by the place of each texel on the surface (`gild_knot`). The mesh grew to 13 282 vertices and the base
  colour changed; skeleton and motion are the same. All FBX files exported again and read back (failed: 0), the
  embedded base colour compared with the repainted one, art page and `README.md` updated. Tool: Claude Code.
- What went wrong: repainting texels alone never gave a clean neck, because the faces in the crease are slivers
  and the painted border zigzags from vert to vert, so the border had to be cut into the mesh. With the head tilted
  far (`Hit_Reaction`) the border behind the ears is still uneven: two more attempts changed nothing there, the
  cause (texture or crumpling mesh) is not confirmed, and the author chose to stop here. Not checked in Unreal.

### 01:30 The 3D models in the game (T-061, T-062)

- What works: the capsules and shapes are gone. The hero and the Publisher are the approved models, 1.8 m and 2.6 m,
  playing their clips; the shuriken, the cash-stack heal, the memos and the bills are the generated props; the
  katana is in the hero's right hand. The hero is exported like the Publisher (`Tools/Art/hero_export.py`, Blender:
  a `root` bone, the travel of `Hit_Reaction`, `Dead`, `Roll_Dodge` moved onto it), both are imported by
  `Tools/Editor/import_models.py` with root lock on the travel clips. No animation blueprint: `UJamPoseInstance`
  blends the idle, the walk and one action in C++, from the spine up while he walks. `JamPoseRules` picks the clip
  from the fight's events and fits it so its impact frame (measured with `Tools/Art/clip_impacts.py`) shows on the
  step the blow lands; 9 `Jam.Pose` tests, red first. The bean, the grid and the T-pose layers work on the model.
  Clip `Clips/t061_t062_models_in_game.mp4` with a new demo switch `-JamDemoSkipEvery=10` (a cut every 10 s, so
  one minute shows every layer). 218 tests. Tool: Claude Code, Blender 5.2 and the editor's Python, headless.
- What went wrong: the merged Meshy clip FBX gave Unreal one take of seven; the Meshy FBX came in 100 times too
  small; the Python edit of the mesh's material slot never saved, so the models first rendered in the default
  material (the game now loads its material by path); the hit flash, a white emission of 2, kept the
  constantly-hit Publisher white for seconds (now 0.4). The demo capture crashed once in the engine when the window
  went off screen and dropped frames when it lost focus. The Publisher's walk still slides: he moves six times
  faster than the clip. The Unreal MCP did not connect; everything ran through headless commandlets.
- Build: Shipping `Pavyny_RoninFinalCut_Session5k.zip`, smoke-run (still running after 30 s), pushed as `session-5k`
  (166 MB patch). The Shipping unity build first failed on a name of ours (`TextScale` in the HUD) clashing with an
  engine header; renamed `HudTextScale`.

### 02:20 The night: the hero's cloth, the hit lean with afterimages, the intro (T-063, T-064, T-030)

- What works: the author answered four questions before going to sleep (apply the cloth fix for a morning review,
  Rodin waits, delete the import leftovers, package but do not push). The soft cyan and red-magenta glow halos Meshy
  baked into the hero's black cloth are painted over with the cloth's black; the piping, the mask and the face are
  protected by the skin weights of their bones (`Tools/Art/hero_cloth_fix.py`, four-side before/after renders with
  `Tools/Art/flat_views.py`). A hit no longer starts an action: the upper body leans additively (rotation only, so
  the legs and a strike under it go on) and the hero leaves four cyan and magenta afterimages that fade over 0.3 s.
  Before the first fight the Publisher shows off with `Kung_Fu_Punch` under a title card, skippable. 223 tests.
  Clips `Clips/t030_intro.mp4`, `Clips/session5l_night.mp4`. Tool: Claude Code, Blender 5.2, the editor's Python.
- What went wrong: the dry run of the cleanup said "none tracked by git", but its check used a wrong pattern: two
  files of `Art/Models/hero_fixed/SK_Hero.fbm` were committed in T-061. They were deleted with the author's yes on
  that wrong count, then restored from git at once; they are back and the decision is the author's. The first
  render comparison showed no change: the view tool's texture swap matched no node, fixed. The parry shield stood
  around both fighters through the whole intro (an untouched parry state reads as raising before the first step;
  it was one frame before). Two more unity-build name clashes (`Length` in a test, `Skin` in the view).
- Build: Shipping `Pavyny_RoninFinalCut_Session5l.zip`, smoke-run (still running after 30 s), **not pushed**: the
  author pushes in the morning.

### 04:00 Polish night: the jumping hero, the blade in the fist, smooth motion, three tiers, the crack (T-065 to T-068)

- What works: the author asked to polish as a partner (plan `Tickets/PLAN_polish_night.md`, grill answers in it)
  and reported that the hero's model jumped to another part of the screen when he moved. The cause was in our hero
  export: Blender keeps an unkeyed bone where the last clip left it, so the new `root` carried the end of
  `Hit_Reaction` into `Idle` (1 m off) and the end of `Roll_Dodge` into `Run`, `Walk` and `Sword_Parry` (4 m off);
  every clip now starts from the rest pose, and the walk and run come from Meshy's full GLBs (the armature-only ones
  left the arms in a T-pose). A pose gallery (`-JamPoseGallery`) holds the hero in twelve poses before a close camera
  and logs the hand bone: the katana's grip was worked out from it and sits in the fist in every pose. Two Meshy
  library clips bought (6 credits, the author's go): the quick cast mirrored onto the left arm is the throw, and
  shots are drawn leaving the throwing hand. Actions crossfade, the Publisher turns at a rate, the hero backpedals,
  a dash leaves the afterimages. Three tiers: the 3D hero and full art, then the shape ronin of before the models,
  then greybox. A change of look tears the screen: the old frame splits along a neon crack while the fight holds.
  227 tests. Tool: Claude Code; the Unreal MCP (on again from 03:00) for the pose work; Blender 5.2.
- What went wrong: the gallery's first grips floated 20 cm up the forearm (the hand bone's +Y runs to the elbow,
  not to the fingers) until the bone was logged instead of guessed. Several unity-build name clashes again. The
  crack at the first cut is subtle because only the hero changes, in the middle where the crack opens first: the
  old halves now slide apart to make it read. A build closes the editor, so the MCP and a build cannot overlap.

### 04:05 Polish night, part two: Niagara effects (T-069), build session-5m

- What works: five Niagara systems made in the editor through the Unreal MCP by a helper agent (sparks, slam dust,
  a ribbon trail, a cash burst, arena embers), picked by `JamFxRules` from the fight's events in the full-art tier
  only; the katana and every shuriken leave a ribbon. 231 tests. Clip `Clips/session5m_polish.mp4` (the intro, then
  a fight through every cut, the cracks and the effects). Tool: Claude Code, a helper agent on the Unreal MCP.
- What went wrong: the first look washed the Publisher white under big bursts and white smoke (sizes capped, smoke
  greyed); a test indexed an empty list and crashed the red run instead of failing it (guarded). The trail system
  was not seen by eye by the agent; it shows on the blade in the game frames.
- Build: Shipping `Pavyny_RoninFinalCut_Session5m.zip`, smoke-run (still running after 30 s), **not pushed**: the
  author pushes after review.

### 13:00 Sunday feedback on session-5m: four fixes, builds 5m and 5n on itch

- The author played session-5m ("it plays great, it is exactly what the game was meant to be") and sent four fixes.
  Session-5m went to itch first, as approved.
- What works: his stolen shurikens and his shuriken burst come 30% slower (T-070, `BossShurikenCooldownScale=1.3`,
  the hero's 0.4 s stays). The crack plays over 2.1 s instead of 0.7 s, along a jagged fault line cut by midpoint
  displacement, the halves tinted cyan and magenta as they slide off (T-071). The Ultimate charge is a Niagara aura on
  the hero in the full art (it grows with the meter, bursts when full), a floor glow on the shape ronin, the bars
  again in greybox; the dash loses its floor bar in the full art (T-072). 236 tests. Clip
  `Clips/t070_t072_sunday_fixes.mp4`. Tool: Claude Code; a helper agent built `NS_JamUltCharge` on the Unreal MCP.
- What went wrong: the first fault line passed its tests but read as an even zigzag on screen (rebuilt by midpoint
  displacement after the look check). The first aura was too faint from the game camera (boosted). The steal card's
  offer is random, so three demo runs never showed him with the shuriken; a demo switch `-JamDemoGive=SHURIKEN` now
  hands it over at the start. Even then the bot fights at close range, where he does not throw it, so his throws are
  rare in the clip; the spacing is covered by the tests. The itch page was renamed to `pavyny/ronin-final-cut` by
  the cover session, so the push script failed with "invalid game" until it was pointed there.
- Builds: session-5m pushed; Shipping `Pavyny_RoninFinalCut_Session5n.zip`, smoke-run (still running after 30 s),
  pushed as session-5n.

### 15:30 Sunday afternoon: three parallel tracks, build session-5o

- The author played session-5n ("everything else is great") and sent one combined task, run in parallel: this
  session orchestrated, two lead agents (Claude Code, Opus) worked in their own git worktrees, one track each, and
  reported back; their branches landed on main one at a time with the full suite after each landing.
- What works:
  - Track A (this session): the Ultimate meter bar is back under the hero's health in every tier; the aura shows only
    when the meter is full and through the wind-up (T-073). The hero throws 33% more shurikens (0.3 s, 18 steps) for
    the same damage a second (6.75), a blast every 4th as the author chose; the Publisher's stolen copy keeps 0.52 s
    and 8.1 through its own base keys, pinned by a test that reads the game's own balance (T-074).
  - Track B (lead B): the full-art tier is a neon night: Lumen GI and reflections (software, no hardware ray
    tracing), 16 cyan and magenta lamps, a dimmed sun, vignette and grade; Niagara hits with a white core and a short
    light, slam and blast shockwave rings, cash sparkles; the flat discs only in the lower tiers (T-075 to T-077).
    Shape ronin falls to flat light and little bloom, greybox to bare. GPU 3.4 ms mean, 8.1 ms max at 1080p on the
    RTX 4070 SUPER of this machine (csvprofile, a bot fight with Ultimates and slams); a mid-range GPU is estimated,
    not measured.
  - Track C (lead C): the hero's legs run where he moves: four Meshy diagonal runs (12 credits) blended by the move
    angle, the hips twist up to 60 degrees with the spine turned back to the aim, play rate follows ground speed, a
    small lean into turns (T-078); a backpedal plays the plain run backwards (T-080, the author's answer). The
    Publisher's walk plays as fast as his feet need (T-079), a quick waddle the author kept.
  - 246 tests, 0 check warnings. Clips: `Clips/t073_t080_three_tiers_same_fight.mp4` (the same fight, seed 7, in the
    three tiers side by side), `Clips/t078_t080_legs_follow_the_move.mp4` (eight directions, circling, backpedal).
- What went wrong: lead B's first look washed the floor grey (the sun under Lumen) and its sparks bloomed white
  over the Publisher, both fixed after frame checks; a cash burst was never seen in a demo run. Lead C found no plain
  strafe or backwards run in the Meshy library, so diagonals carry the strafes; foot slip at the diagonals is likely
  but not measured. The environment and voice-over sessions worked on the same machine: ownership was split by
  message (arena view and materials to the environment session, lights to lead B), and main's working tree held
  another session's half-written code, so the final build was made from a clean worktree of main HEAD. The fight's
  random seed made "the same moment in all tiers" impossible until a `-JamSeed` demo switch; the lower tiers open
  with a crack that holds the fight 2.1 s, trimmed from their clips. CastleFight's packaging held Unreal's one
  AutomationTool for a while.
- Build: the voice-over session landed its work on main on the author's word (a74e401, with the cover art), and the
  author chose to include it. Full suite at a74e401 (250 tests, 0 warnings), a capture with the subtitles, then
  Shipping `Pavyny_RoninFinalCut_Session5o.zip` from a clean worktree of a74e401, smoke-run (still running after
  30 s), pushed as session-5o to pavyny/ronin-final-cut.

### 17:30 The rooftop shrine: the arena of the full art (T-081)

- What works: the author found the arena too plain for the full art. Three concepts from the game camera (fal, GPT
  Image 2.6): a rooftop shrine, the Publisher's boardroom and a floating zen garden; the author chose the shrine.
  Prop sheets from fal and five Meshy models (lantern pillar, torii, stone lantern, bonsai, railing; 150 credits),
  a wet slate floor from PBRgen (the author's tool, 18 credits) with puddles in the low spots and an engraved crest,
  the city far below, rain and floor ripples, a splash under each fighter's stride. The first cut leaves the bare
  shrine (slate and pillars), the second the grid. With lead B's neon lights the wet floor catches cyan and magenta
  pools. 252 tests (two new, red first). Tools: Claude Code in its own worktree; a helper agent built the rain
  systems on the Unreal MCP; assets by a headless editor script.
- What went wrong: the PBRgen tab froze while Chrome was in the background and its download landed in the author's
  asset folder, not Downloads, so the floor waited on the author. The first rain followed the spec (1.5 cm streaks)
  and could not be seen from the camera, and drops spawned high passed the camera as huge streaks (bigger, lower,
  culled near the camera). The first slate was too bright and busy under the fight (darkened, tiles doubled). The
  ticket number T-080 was taken by a parallel track while this ran (renumbered T-081). The footstep splashes are
  still faint.

### 16:00 Quick fixes on session-5o, build session-5p (T-082 to T-085)

- What works: the author's hour of quick fixes. A warm key light follows the Publisher in the full art, the night is
  lifted (arena sun 25%, lamps 55 cd, exposure +0.5), and the coloured glow pools under both fighters are gone
  (T-082). The Publisher's health bar is hidden through the intro and fades in over 0.6 s when the fight starts
  (T-083). A throw while backpedalling no longer twists the hero's upper body away: the hip twist fades out under an
  upper-body action (T-084). The crack's sound plays once, at 0.3 (0.12 under a voice line), cut after 0.35 s, and
  the crack line is thinner (T-085). 255 tests, three new, red first. Tools: Claude Code.
- What went wrong: the crack's sound played on every frame of the crack hold, because the held fight kept the last
  frame's steal event (a held frame now clears its events). The first key light (9000 cd) washed the Publisher white;
  1100 cd keeps his hands and shoes readable. The backpedalling throw is checked by the rule test only, not by eye.
- Build: full suite in a clean worktree of main HEAD 86c9628 (shrine arena, voice-over, tracks B and C, these fixes;
  255 tests, 0 warnings), Shipping `Pavyny_RoninFinalCut_Session5p.zip`, smoke-run (still running after 30 s), pushed
  as session-5p to pavyny/ronin-final-cut.

### 17:15 The throw's real cause, a hand on the spot, the hero's draw, build session-5q (T-086 to T-090)

- What works: the author played 5p and asked for checks by data, not by game windows. The throw that folded the
  torso (T-084 had not fixed it) was traced in headless Blender: the Meshy rig's centre bones have their X axis up,
  so the T-065 mirror (`flip @ partner @ flip`) turned every bone of Throw_L about 180 degrees about its own length
  (the pelvis 178 degrees off rest, 59 in the source cast); the skin twisted, and a crossfade lerped the spine through
  it. The mirror now reflects each partner's change from rest (0.0 degrees off the true mirror), and Throw_L was
  exported again (T-086). The Publisher's spot is a fifth wider and trails him on a critically damped spring, half
  a second behind, with a slow sway (T-087). Flying shurikens light the floor in their colour, up to six at once
  (T-088). The hero draws his blade as the intro's camera comes to him, a Meshy library clip (Sword_Shout's opening,
  1.5 s), and takes control after it (T-089). The Publisher's show-off is turned 120 degrees counter-clockwise so it
  ends facing the hero: the author's 45 was measured short, the clip ends 98 (head) to 156 (chest) degrees off his
  idle (T-090). 263 tests, eight new, red first. Tools: Claude Code, headless Blender, Meshy API (9 credits).
- What went wrong: the Meshy library has no sword draw, so three clips were bought to compare. Two false alarms
  cost time: the analysis first sampled the glTF at 24 fps (the frame numbers were off), and an armature-only FBX
  takes its rest from the pose at export, so "off rest" read from it means nothing; world space settled both. The
  new tests' file-level names clashed in the unity build of the clean release worktree (renamed). A commit swept in
  four untracked briefs from Tickets/ (PROMPT_*.md). Nothing here was checked in a game window, by the author's ask.
- Build: Shipping `Pavyny_RoninFinalCut_Session5q.zip` from a clean worktree of main 2026cf1 (263 tests, 0
  warnings), no smoke run, pushed as session-5q to pavyny/ronin-final-cut.

### 17:40 The Publisher comments on what he stole, build session-5r (T-091)

- What works: the author wrote five new lines: after a cut, once the crack has closed and the card's two lines are
  said, the Publisher comments on the move he took (shuriken, parry, dash with Ronin's answer, Ultimate; the Katana
  has none). Voiced with ElevenLabs on the saved voices (about 1 dB from the approved lines' level), imported headless,
  with subtitles. `JamVoiceRules::CommentsOf` and `Release` decide which line and when; barks wait while a comment is
  held. 265 tests, two new, red first. Tools: Claude Code, ElevenLabs API, the engine's ImportAssets commandlet.
- What went wrong: the author's cue names (`vo_steal_parry`) broke the voice test's rule that every Publisher file ends
  in `_pub`; the check now tests only what the code reads (`_ron` is the hero). A `python -` waited on stdin again.
  Not heard in a game window, by the author's ask.
- Build: Shipping `Pavyny_RoninFinalCut_Session5r.zip` from a clean worktree of main b76c410 (265 tests, 0 warnings),
  no smoke run, pushed as session-5r to pavyny/ronin-final-cut (build 2395605).

### 18:10 The main menu and the opening comic (T-092, T-093)

- What works: a main menu before the intro: NEW GAME, SETTINGS (graphics quality, window mode, resolution, master,
  music and voice volume), EXIT, by keys or mouse, over a live shot of the hero and the Publisher idling face to face
  in front of a generated skyline (T-092). After NEW GAME a five-panel comic tells why they fight: the Publisher calls
  the dev at 3:47 AM, the game ships today, he will cut what he does not need himself. Pictures by fal GPT Image 2.6
  (panels 2 to 5 edited from panel 1 and the character sheets so the faces hold), four lines voiced with ElevenLabs,
  balloons, captions and camera moves drawn by the game, skippable (T-093). 275 tests, ten new, red first. Tools:
  Claude Code, fal API, ElevenLabs API, headless editor imports.
- What went wrong: the fal spend cap counts the whole ledger, so a round was refused until it was raised. The menu's
  skyline plane was stood the right way from its UVs read headless. Three functions went over 60 lines and were
  split. The comic's crop test first aimed at a corner, where the clamp hides the move. Neither the menu nor the comic
  was seen in a game window, by the author's ask.

### 18:50 The author's fix list for the menu, the comic and the card (T-094)

- What works: the menu's skyline is outpainted to a 3:1 panorama (fal GPT Image 2.6) and placed each frame to cover
  any screen with the tower in frame; the two fighters keep idling. Settings sit on an opaque panel with dropdowns for
  quality, window and resolution and sliders for the volumes, where the drawn controls are what the mouse hits. Voice
  lines play 30% louder. A new soft ambient loop (ElevenLabs Music, pitched to the theme's C by our script) plays under
  the comic and the intro and crossfades into the theme when the Publisher's line ends or the player skips. The comic
  keeps 16:9 with black bars. The steal card takes a click and pops in with an overshoot, the stun and ending bands
  too. The full art gains a light film look: grain, rain haze, a colour fringe at the edges, a soft far distance. 286
  tests, eleven new and two changed, red first. Tools: Claude Code, fal API, ElevenLabs API, ffmpeg, headless editor
  imports.
- What went wrong: fal refused a 3.11:1 outpaint (3:1 is its limit). ElevenLabs ignored the key in the prompt and
  gave a drone on A. The headless import failed while the editor was open, so the editor was closed first. A wrong
  float tie left a hairline strip on a 5:1 screen until a 1% overscan. The first crossfade made Audio include Game,
  which the layering check caught; the fade moved into the game mode. Nothing was looked at in a game window, by the
  author's ask.
- Build: Shipping `Pavyny_RoninFinalCut_Session5s.zip` from a clean worktree of main 2ef4d83 (286 tests, 0
  warnings), no smoke run, pushed as session-5s to pavyny/ronin-final-cut (build 2396268). The first package failed:
  the release worktree's unity build put the slider's `BarHeight` next to the HUD's constant (C4459), which the main
  tree's adaptive build had compiled alone; renamed in 2ef4d83.

### 19:45 The author's fix list after session-5s: comic, banter, the hero's tier, the rune telegraph (T-095)

- What was done: the comic's first panel fades up and pushes in slowly for 7 s over mechanical keyboard typing, the
  call panel rings before the Publisher speaks. Eight pairs of banter (the author's text, the same two ElevenLabs
  voices) fill a fight quiet for 5 s, sharing one no-repeat bag with the old barks; event lines cut them off. The
  hero keeps his 3D model and his dash's afterimages until the second cut. Before the first cut the slam's circles
  are a rune seal exactly the size of the hit (a fal texture on our own additive material) that lights from the rim
  inward to the impact, with embers rising off the rim (Niagara, made through the editor's toolset) and a hot spark
  burst on landing; after the first cut the plain red disc as before. 294 tests, eight new and four changed, red
  first. Tools: Claude Code, ElevenLabs API, fal API, headless editor imports, the Unreal MCP's Niagara toolset.
- What went wrong: the comic's old 20 s cap failed once the first panel held longer; the cap became 25 s. Nothing
  was looked at in a game window, by the author's ask.
- Build: Shipping `Pavyny_RoninFinalCut_Session5t.zip` from a clean worktree of main bfce49e (294 tests, 0
  warnings), no smoke run, pushed as session-5t to pavyny/ronin-final-cut (build 2396684). The first package failed:
  the new slam test sat after its file's `#endif` of the automation guard, so Shipping compiled it without the test
  helpers; moved inside in bfce49e.

### 20:05 The Publisher's shirt under the tie, plain black (art, parallel session)

- The main session should reimport the Publisher with `Tools/Editor/import_models.py` (JAM_IMPORT_ONLY=Publisher, with its textures) before its next package: the shirt under the tie no longer shows a grey imprint (UVs moved off a seam texel, `Tools/Art/publisher_rig/plain_shirt.py`, Claude Code, headless Blender).

### 20:25 HOW TO PLAY, the itch page images, the last look fixes (T-096)

- Done: a HOW TO PLAY screen drawn by the HUD, a neon keyboard with only our keys lit and leader lines to labels that
  come from the bindings in code (one shared `JamKeys`), shown once after the comic with a 15 s countdown and on F1
  over the fight, which waits. The author found the drawn mouse odd: it is now a generated neon mouse (fal, GPT Image
  2.6) under native labels, as in the comic. The comic's first panel is 5 s instead of 7, the theme comes in halfway
  through the Publisher's kung fu, and the Publisher is imported again with the shirt fix (its repainted base colour
  needed its own import). The itch page got the Publisher's letter as its hook, the katana fact (never on a card,
  only the final note takes it), the controls image and a banner composited from two real frames and the cover's
  logo; five real screenshots. 299 tests, five new and two changed, red first. Tools: Claude Code, fal API,
  headless editor imports, PIL.
- What went wrong: the F: drive filled up mid-session (a file write failed); space came back without a deletion.
  The first captures were cluttered (lines over caps, glow on every key); three rounds of fixes, each checked on a
  captured frame.

### 20:45 The author's final play-through fixes (T-097)

- Done: the comic's first panel is 2 s of typing before the ring; a glass shatter (ElevenLabs, trimmed so its hit
  lands on the frame the panel appears) under the Publisher smashing through the screen; his kung fu show turned 10
  degrees more clockwise; the theme now rises from silence under his show and is full as the camera goes to the hero,
  or crossfades in from a skip. The Publisher's shirt fix was already in from T-096. 299 tests, red first. Tools:
  Claude Code, ElevenLabs API, headless editor import.
- What went wrong: session-5u's first Shipping package failed on a unity-build name clash (a file-scope `Body` in the
  HOW TO PLAY layout hid a parameter in JamTeachRules); renamed in 8d91e42. Session-5u was packaged but not pushed:
  these fixes came first, so session-5v is the next build on itch.
- Build: Shipping `Pavyny_RoninFinalCut_Session5v.zip` from a clean worktree of main 964ce46 (299 tests, 0 warnings),
  no smoke run, pushed as session-5v to pavyny/ronin-final-cut (build 2397186).

### 20:55 The game hung off the screen (T-098)

- Done: the author's run of session-5v showed only the top-left of the game. Its saved settings had a 3840x2160
  window, larger than the desktop. A window is now never larger than 90% of the desktop: picked in the menu or
  found in the saved settings at launch, which mends the author's file on the next run. 300 tests, one new, red
  first. Tools: Claude Code.
- What went wrong: the monitor's size and Windows' scaling could not be read from the agent's shell (the query was
  refused by its permission check), so the fix covers the window's size whatever the screen; not run in a window.
- Build: Shipping `Pavyny_RoninFinalCut_Session5w.zip` from a clean worktree of main 7a56372 (300 tests, 0 warnings),
  no smoke run, pushed as session-5w to pavyny/ronin-final-cut (build 2397273).

### 21:20 Rodin props, the fight after the first cut, the menu, crisp UI (T-099)

- Done: the two approved Hyper3D Rodin models in the game, decimated to about 9k triangles in headless Blender: the
  cash-stack heal and the full-art arena's stone lanterns. The hero hits 20% harder from the first cut on. The menu's
  fighters face each other from its first frame. The HUD's text is rasterised at its real size (it was a 10-point font
  stretched) and the whole HUD grows with the screen, up to ultrawide and 4K. 303 tests, three new, red first. Tools:
  Claude Code, Blender 5.2.
- What went wrong: glTF imports land in a folder named after the file, so the cash stack's path changed; the Rodin
  lantern's windows were too dark to glow and were brightened.
- Build: Shipping `Pavyny_RoninFinalCut_Session5x.zip` from a clean worktree of main 15e4d26 (303 tests, 0 warnings),
  no smoke run, pushed as session-5x to pavyny/ronin-final-cut (build 2397610).

### 22:10 The hero's spotlight, an easier fight, the old cash stack (T-100)

- Done: the hero has his own small warm spotlight in the full art; in the intro it splits off the Publisher's beam and
  rides the camera onto him, then trails him like the Publisher's. The hero has 120 health (was 100) and hits 1.4 times
  as hard after the first cut (was 1.2): the bot still wins 60 of 60, its median fight 80 to 69 s, its closest win 63 to
  72 health. The heal is the Hunyuan3D cash stack again; the Rodin lanterns stay. 305 tests, two new, red first. Tools:
  Claude Code.
- What went wrong: the bot wins every fight, so its win rate cannot show an easier fight; length and closest win do.
  1.5 broke the author's fight-length floor, so 1.4. A slam no longer takes a third of the hero; its guard asks a quarter.
- Build: Shipping `Pavyny_RoninFinalCut_Session5y.zip` from a clean worktree of main 3c44c35 (305 tests, 0 warnings),
  no smoke run, pushed as session-5y to pavyny/ronin-final-cut (build 2397948).

### 22:25 The comic's glass was not in the package (T-100)

- Done: the author heard no glass on the gauntlet's panel. The cooked asset registry of the build had every audio asset
  but `sfx_comic_shatter_01`: the sounds are loaded by path and `/Game/Audio` was not always cooked. It is now, and the
  glass plays at 0.6, under the Publisher's line. 307 tests, two new, red first. Tools: Claude Code.
- What went wrong: T-097 tested the glass in the editor, where every asset loads; the package was never checked for it.
- Build: Shipping `Pavyny_RoninFinalCut_Session5z.zip` from a clean worktree of main 352600a (307 tests, 0 warnings),
  the glass checked in its cooked asset registry, no smoke run, pushed as session-5z to pavyny/ronin-final-cut (build 2398116).
- Clean-up on the author's yes: the unused Rodin cash-stack assets and base_basic_shaded.glb deleted after a dry run
  (6 files, 42.6 MB); nothing referenced them.

### 21:55 Difficulty levels (T-101)

- Done: a DIFFICULTY row under NEW GAME, `< NORMAL >`, cycled by the side arrows, Enter or its drawn arrows, saved
  with the volumes. A level scales numbers the fight already has (hero health, his damage, the cash stack's heal, the
  Publisher's damage), one block per level in `[JamBalance]`; NORMAL is all ones. A retry and PLAY AGAIN keep it. The
  bot wins 60 of 60 on every level: EASY's closest win 148 of 180 health, NORMAL 72 of 120 as before, HARD 4 of 96, so
  the author's starting scales stay. 312 tests, five new, red first. Tools: Claude Code.
- What went wrong: one retry test passed against the stub because the stub's scales were all ones; it now also checks
  that EASY differs from NORMAL.
- Build: Shipping `Pavyny_RoninFinalCut_Session6a.zip` from a clean worktree of main bd01abf (312 tests, 0 warnings),
  no smoke run, pushed as session-6a to pavyny/ronin-final-cut (build 2399032). The first package failed: a new
  local, `LabelScale`, hid the how-to's constant in the Shipping unity build; renamed.

## Mon 16 Nov

### 00:35 NVIDIA DLSS 4.5 Super Resolution (branch dlss)

- Done: the NVIDIA DLSS 4.5 plugin v8.9.0 for UE 5.8 (only `DLSS` and `StreamlineNGXCommon`, no debug symbols) in
  `Plugins/`, the NGX runtime through Git LFS. SETTINGS has an NVIDIA DLSS row under RESOLUTION: OFF, QUALITY,
  BALANCED, PERFORMANCE, QUALITY on a fresh install on an RTX card, saved with the volumes. Where DLSS is not supported
  (or with `-NoDlss`) the row offers only OFF and nothing is touched. 315 tests, three new, red first. Tools: Claude Code.
- Checked in a packaged build of the branch on the RTX 4070 SUPER: the Development log says `DLSS Super Resolution
  on: mode 1, screen percentage 66.7, r.NGX.DLSS.Enable 1` and NGX creates the DLSS-SR feature with preset K;
  with `-NoDlss` it says `not supported, off` and the game runs. The Shipping package starts both ways.
- What went wrong: the author was playing the build in the release worktree, so the branch was packaged in a new clean
  worktree (`MegaJam2026_dlssrel`); its first cook failed until the editor was built there. The root `Binaries/`
  ignore rule hid the plugin's NGX DLLs from git; it now covers the root folder only.

### 01:10 A sharp first launch, the final Development build (T-102)

- Done: the blurry first launch had two causes. The game was not DPI aware, so Windows told it the author's 3440x1440
  screen at 125% was 2752x1152 and stretched the picture; and a fresh install left `sg.ResolutionQuality` at 0, so TSR
  rendered at the engine's display-based default, about 72%. `bAllowHighDPIInGameMode` is on, and
  `JamDisplayRules::Settle` starts a first launch borderless at the desktop's size, keeps a borderless window at the
  desktop's size and renders the scene at 100% (DLSS sets its own percentage on RTX cards). 318 tests, three new, red
  first. Tools: Claude Code.
- Development build for the jam: F5/F6 and their hint are editor-only; a packaged build turns off engine screen messages
  and the console key. `-JamFrameLog=<file>` writes the average frame rate.
- Checked on first launches with the author's settings file renamed and put back: before, borderless 2752x1152 at about
  72%; after, borderless 3440x1440, DLSS QUALITY at 66.7% on the RTX 4070 SUPER, 100% with `-NoDlss`. A 50 s bot fight
  at 3440x1440: Development 155 fps with DLSS and 118 without, Shipping 155 and 120. No test, demo or bot mode runs
  without its flag; no debug text in 420 captured fight frames (menu, comic and end screen not captured).
- What went wrong: T-102 set the render scale through the user settings at a priority above the one DLSS used, so my
  first final zip (32f6db1) rendered at 100% with DLSS on; the DLSS session found and fixed it (94c554d). Its
  Development package of 94c554d (Session6d) is the final: `Pavyny_RONIN_FINAL_CUT.zip`, pushed to the itch channel
  `windows-final`. itch names a butler upload `<slug>-<channel>.zip`, so the author sets the display name and hides
  the old `windows` upload in Edit game.
- `Submission/openai_answer.md`: how GPT Image 2.6 (through fal) was used, checked against THIRD_PARTY.md and the art
  ledgers.

### 01:25 The slate floor's relief (T-103)

- Done: the slate read flat, most of all under the menu's low camera. Material only: `NormalStrength` 2.5,
  `CavityDarken` 0.6 (the stones' gaps darker), `ParallaxHeight` 0.02 (a world-space offset by the height map, almost
  none from the fight camera). In the menu a 6 lux cool rim light 7 degrees over the horizon, on lighting channel 1,
  which only the slate takes, so the fighters keep their look. The cracks' glow and the puddles are untouched.
  Tools: Claude Code.
- Checked: before (Session6d) and after (the packaged 0ad4d2b) at the same frames, the menu at 3440x1440 and the
  full-art fight at frame 150 of the bot's seed-7 run (`floor/before_*.png`, `floor/after_*.png` in the session's
  scratchpad). `-JamCapture` now saves the menu's frames too.
- What went wrong: the menu opened behind the browser, so the first grab caught the browser; the saved display mode
  overrides `-windowed`, so the menu grabs take the whole desktop of a packaged build.

### 01:55 Neon bars in the full art (T-104)

- Done: the approved mock (`Submission/hud_mock_neon.png`) drawn on the HUD's canvas, no textures: slanted bars on a
  dark plate with scanlines, segmented fills with a bright top edge, a neon frame with a soft glow. The Publisher's bar
  pink-red with his name tab and cut marks where he steals (76, 52 and 27% in this balance), Ronin's cyan with a
  magenta Ultimate bar that pulses when full. The author asked for them to last two stages: they go with the hero's
  own look at the second cut (`JamNeonBarRules::Shows`), then the plain bars. `JamNeonBarRules`, five tests, red first;
  323 tests. Tools: Claude Code.
- After the author's play-through: the frame and the segments leaned at different angles (one pixel offset for shapes
  of different heights). Now every shape shifts by its distance from the bar's bottom line times one slope
  (`JamNeonBarRules::Slanted`, a parallel-edges test). The chunk of health just lost read grey: it is white at 0.9 and
  cut into segments like the fill.
- What went wrong: canvas line items drew a faint colour opaque and the additive blend ignored alpha, so the first try
  flooded the screen; every shape is now translucent triangles. The package's unity build joined the new file's
  unnamed namespace with other HUD files and the names clashed; they all carry a `Neon` prefix.

### 02:05 The final build: 79e93f3, uploaded by hand

- Done: the Development package of 79e93f3 (the floor's relief, the neon bars with one slope, the bright segmented
  chunk, the bars through the first cut) is `Pavyny_RONIN_FINAL_CUT.zip`, 574,266,855 bytes, zipped 01:59:58. Its
  provenance: the release worktree's HEAD and the package script's log both name 79e93f3, and the zip's
  `JamGame/Binaries/Win64/JamGame.exe` is from 01:58, that build's. The author uploaded it to itch by hand.
  Tools: Claude Code.
- Checked: 323 tests green, 0 check warnings; one bot fight (seed 7, 1920x1080) from the package, and the author's
  approval.
- What went wrong: the butler push to `windows-final` was refused while the channel's earlier build (#2399864, the
  floor-only a74eeba) was still processing; the retry loop was stopped on the author's word after three refusals, and
  `butler status` shows no build from it.
