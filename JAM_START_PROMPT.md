# Jam start prompt

Opening prompt of the first jam session. Type:
"read F:\Unreal_Engine\UnrealProjects\MegaJam2026\JAM_START_PROMPT.md and follow it".

## If the session runs from another folder

The author works through a remote session that is bound to the CastleFight folder, so the first jam session may
start there after `/clear`. In that case:

- The jam project is `F:\Unreal_Engine\UnrealProjects\MegaJam2026`. Use absolute paths and `cd` there for every
  git and build command. Touch nothing in CastleFight.
- The project rules of this session are the jam's `CLAUDE.md` and `.claude/rules/cpp.md`, read by hand. The
  CastleFight project rules and tickets do not apply; the author's global rules do.
- The log exporter only finds sessions started from the jam folder. Before the first commit add this session to
  `Tools/Devlog/sessions.json` as `{"slug": "F--Unreal-Engine-UnrealProjects-CastleFight", "session": "<id>",
  "note": "(first jam session, run from the CastleFight folder)"}`. The id is the session folder in your
  scratchpad path. After the commit check that `_AgentLogs/INDEX.md` lists it. Do the same for every later session
  that starts outside the jam folder.

## Before anything else

1. Check the clock: `date -u`. The jam window opens at 2026-11-12 05:00 UTC. Before that moment write no gameplay
   code, levels, art or sound. Tooling and tickets are allowed.
2. Read `CLAUDE.md`, `JAM_PLAN.md` and `.claude/rules/cpp.md`. Open `JAM_BRIEF.md` only for the section you need.
   `JAM_PLAN.md` is written in Russian; it is the agreed design and schedule.
3. Talk to the author in English, short: answer first, 3 to 5 lines, one next action. The session log is published
   as part of the dev log, so the conversation, code, comments, commits and every text in the game are in English,
   even when the author writes in Russian.

## Who is where

- The author (Pavlo, Team Owner) is away from the home PC until the evening of 13 November and follows from a phone.
  Send him concept images, clips and questions as files. Questions that need his eyes go to `QUESTIONS.md`; do not
  stop the work for them: take the default and go on.
- Kirill delivers music, sound and UI design through a Google Drive folder. Nobody but the agent touches git or
  the editor.

## The game in five lines

- One arena, one enemy: the Publisher. The whole game is a boss fight of 3 to 4 minutes. No waves, no other enemies,
  no auto-attack.
- At boss health thresholds the Publisher grabs two random mechanics, the player picks which one to give away, and
  the boss uses it against the player for the rest of the fight.
- Each steal also strips a cosmetic layer from the world and the hero (VFX, textures, animations, model, music
  stems). The boss stays polished.
- Below 20 percent the player is a bean with a weak ball shot. At 0 the boss is stunned, everything returns at once
  and the player lands the final blow with the full Ultimate.
- Win title: "NEON RONIN: DIRECTOR'S CUT". Loss: "SHIPPED AS PROOF OF CONCEPT", retry from the last steal.

## First half hour

1. Add this session to `Tools/Devlog/sessions.json` (see above). Create `DEVLOG.md` with the first entry: jam start,
   the plan in two lines, the tools in use.
2. Turn blocks A, B and C of section 5 of `JAM_PLAN.md` into tickets, one file per ticket in `Tickets/`, each with
   its test seam. Blocks D to G get one-line stubs; they are detailed on Friday morning.
3. Start the concept round in the background through fal (`FAL_KEY` in `.env`, never print it; the key has not been
   verified by a request yet): check the current OpenAI image endpoint in the fal documentation first, then
   generate the hero, the boss (a corporate samurai in a suit, same proportions as the hero, 1.5 to 2 times
   larger) and the arena mood. Empty hands on characters, weapons separately. Spend at most 5 USD on this round.
   Send the images to the author. Do not wait for them: the prototype runs on beans.
4. Install butler. Push the empty Shipping build to the itch draft once the author sends the page address
   (`BUTLER_API_KEY` is in `.env`, the account is `Pavyny`). Check that the name NEON RONIN is free. Read one WAV
   from the Drive folder once the folder exists.

## Today: the prototype on beans, playable build at 23:00

One session, no subagents, test first. Order (section 5 of the plan):

- **Block A.** Arena, the bean moves, an aim abstraction, three cameras switched with F1 (top-down 3/4 like Hades),
  F2 (third person over the shoulder), F3 (first person), the weak ball shot, a boss dummy with health. The
  simulation does not know which camera is active: only the camera and the aim source change.
- **Block B.** Katana, Shuriken and Dash as components with an owner, so that the hero and the boss run the same
  code. Boss AI: chase, the memo volley (projectiles) and the telegraphed area slam.
- **Block C.** Steals at health thresholds with the choice card (3-mechanic version: a pair at 75 and 50 percent,
  the rest at 25), the boss uses what he stole, retry from the last steal, win and loss as text.
- **Then** a bot playthrough, package Shipping, push to the itch draft, one clip per camera. If time is left:
  Parry and Ultimate from block D.

Build from the bean up: the proof-of-concept layer first, mechanics and cosmetic layers on top as switches, so that
every steal is a switch that already exists. Damage, health and thresholds live in one data table.

The author plays the 23:00 build tonight and picks one camera by Friday 07:00 PC clock. Friday starts with block D
on the chosen camera; the other two are switched off. Do not order hero animations before the choice.

## Standing rules

- Test first: a `Jam.<Area>.<Case>` test before the code, decisions in pure `*Rules`. Pick the seams yourself.
- After each block: build and tests, commit, a 30 to 60 second clip, three lines to the author and to `DEVLOG.md`.
- Never amend a commit. Package Shipping every evening. Warn before a run that opens a game window.
- Before a wave of subagents or any workflow: a forecast in chat (agents, tokens, share of the limit, a cheap
  variant) and the author's explicit yes.
- Third-party content goes to `THIRD_PARTY.md` when it is added. Every AI tool used goes to the tool list in
  `DEVLOG.md`.
- Never print API keys, and keep the partner perk codes out of every file and log.
- Hard checkpoints (PC clock, UTC+2): playable build Thu 23:00, test build Fri 18:00, new features stop Sun 12:00
  (moved from Sat 22:00 by the author on Sat 18:30; every milestone ends with a Shipping build on itch),
  video Sun 18:00, submit Sun 20:00 and never later than 21:59.
