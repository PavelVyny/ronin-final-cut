# JAM_PLAN: Epic AI MegaJam 2026

> Translated from Russian on 2026-11-17, after the jam. The original text (Russian with a few English lines added
> during the jam) is in the first commit of this repository. Nothing was added or removed, except that a line-number
> reference to `JAM_BRIEF.md` now points to the section, because the translation shifted the lines.

Version 2 of 12.11, 13:20: the concept is simplified to a single boss fight. The first version (robot waves, timed revisions) is in the git history. Rules and prizes: `JAM_BRIEF.md`. What was in the repository before the start: `PREJAM.md`.

All times below are by the home PC clock (UTC+2). For Oslo, minus one hour.

| Event | Time |
|---|---|
| Jam start | Thu 12.11, 07:00 |
| Epic deadline | Mon 16.11, 03:59 |
| Our submission, hard limit | Sun 15.11, 21:59 (6 hours early) |

## 1. Concept

**NEON RONIN** (working name, uniqueness not checked) is a single boss fight. The player here is the Director, the author of a game about a neon samurai. The Publisher comes into his game in person: "too expensive and too slow". He does not cut features, he takes them for himself: he grabs two mechanics, the player decides which one to give up, and from then on the Publisher hits the player with it. With every theft the game around gets cheaper: effects, textures, animations, music layers go away, while the polished Publisher stays the only expensive object in the frame. The player finishes the last fifth of the fight as a bean with little balls. At zero everything comes back at once, and the player lands the final blow himself. Victory: ": DIRECTOR'S CUT" appears on the title. Defeat: ": PROOF OF CONCEPT".

The jam theme is the mechanic here: the player decides what goes under the knife and takes his version back himself.

## 2. Core loop

1. One arena, one enemy. No waves and no other enemies. No auto-attacks: the player does everything.
2. The player hits the boss, dodges his attacks and charges the ultimate.
3. At the boss's health thresholds the fight slows down for 3 seconds: the Publisher grabs two random mechanics, the player chooses which one to give up.
4. The given-up mechanic passes to the boss, who uses it until the end of the fight. A picture layer and a music track go with it.
5. Below 20% the player is left as a bean. At zero the boss is stunned, everything comes back, the player presses the ultimate: the final blow and the title.

### Fight flow (3–4 minutes)

| Boss health | What happens |
|---|---|
| 100% | The full game: five mechanics, full mix. A Publisher line, the fight starts right away |
| 80%, 60%, 40% | A theft with a choice from a pair. A cosmetic layer and a music track go away |
| 20% | FINAL NOTE "cut the rest": he takes what is left without a choice. Bean, grey world, PoC music |
| 0% | The boss is stunned, everything comes back at once, full mix. The player presses the ultimate: the final blow |
| after | Victory title |

Three-mechanic version (built first): choices at 75% and 50%, the rest goes at 25%.

**Player death:** the "SHIPPED AS PROOF OF CONCEPT" screen and Retry from the last theft (boss health at the threshold, the hero at full health, the mechanic set as after the theft).

### Mechanics

| Mechanic | With the player | Button | With the boss after the theft |
|---|---|---|---|
| Katana | A combo of strikes towards the aim | LMB | Closes in and slashes in a combo |
| Shuriken | A throw, every third one explodes | RMB | Throws from a distance, every third one explodes |
| Dash | A dash with invulnerability | Space | Dashes away from strikes and closes the distance |
| Parry | Reflects a projectile back, on timing | Q | Takes a stance and reflects the player's projectiles |
| Ultimate | A meter from hits, slow motion and a slash across the screen | R | A slash along a line with a warning, step off the line |

- If Katana or Shuriken is stolen, that button fires a weak ball. The player always has an attack.
- The boss's own attacks (there from the start): a volley of memos (projectiles, can be reflected) and an area strike with a warning circle.
- Each mechanic is a component with an owner. A theft is a change of owner: the hero and the boss share the same code.
- Aim: the mouse (in the top-down view it is a cursor on the arena), the right stick on a gamepad.

### Cosmetics

The world and the hero degrade. The Publisher stays at full quality: he is not part of the game's budget.

| Theft | Visuals | Music |
|---|---|---|
| 1 | Effects off | minus a track |
| 2 | Textures turn into a grid | minus a track |
| 3 | Animations turn into a T-pose | minus a track |
| Final | The hero turns into a bean, the interface is cut | PoC version (metronome) |

### Camera

The prototype has three options, switched with F1/F2/F3: top-down 3/4 like Hades, third person over the shoulder, first person. The simulation does not know about the camera, only the camera and the aim source change.

**Pavel plays build K1 tonight and picks one camera by Fri 07:00.** From Friday art and balance follow it. No hero animations are ordered before the choice. In first person you cannot see the hero turn into a bean: take that into account when choosing.

### Intro

Not in the prototype: the game starts with a Publisher line. The idea (a camera from over the developer's shoulder flies into the monitor) is decided on Saturday: a painting with a 5–8 second push-in or a 3D scene. In any form the intro can be skipped with a key.

## 3. Scope

### Full version (the goal)

- One neon arena, hero and boss on one skeleton (the boss 1.5–2 times bigger, a corporate samurai in a suit), one animation set for both.
- Five mechanics, three choices and the final theft, four cosmetic layers.
- The finishing blow with everything coming back, two titles, English text, layered music, sounds.

### Cut line (cut from the top down)

1. Intro.
2. DLSS and the RTX Award.
3. The boss uses the stolen Parry and Ultimate (he just takes them).
4. Mechanics 4 and 5: Katana, Shuriken, Dash and two choices remain.
5. The second and third camera (they go after the choice anyway).
6. The boss uses stolen mechanics at all (he just takes them).

### Never cut

- The path from the full game to the bean, at least four layers.
- The player's choice from a pair.
- English text in the game.
- A packaged Shipping build and the video.
- Submission 6 hours before the deadline.

### If time allows (in order of adding)

1. The intro and the camera pulling back from the monitor on victory.
2. The Publisher's voice.
3. DLSS for the RTX Award (check the country list first).
4. Gamepad.

## 4. Who does what

| Who | Area | Delivers |
|---|---|---|
| **Pavel** (Team Owner) | Direction and acceptance: camera, concepts, models, the feel of the fight. Models in Tripo by hand, two props in Rodin. Video editing on Sunday. The submission form, the itch account | Decisions the same evening |
| **Kirill** | Music in four tracks plus a PoC version, sounds, interface design in Figma (theft card, boss bar, titles). Playtest on Windows | Files in a Google Drive folder, no git and no Unreal |
| **Evgeny** (not confirmed, maybe from 14.11) | Playtest, bug list, a run on a clean machine, English proofreading | A list in the shared chat. The plan does not depend on him |
| **Claude** | All code through tests, art and sound import, builds, itch uploads, raw clips, dev log, the tool and third-party asset lists | A commit, a clip and three lines after each block |

What is needed from Kirill, format-wise:
- Music: 4 tracks of the same length and tempo, looped, WAV 48 kHz. Plus a PoC loop, an accent for the comeback, victory and defeat accents.
- Sounds: katana strike, throw, explosion, dash, parry, ultimate, hit on the boss, damage to the hero, theft (accent), choice card, everything coming back, the final blow.
- File names: `mus_main_1_drums.wav`, `sfx_katana_slash_01.wav`, `ui_steal_card.png`.

Character pipeline: concepts as images through fal, then models in Meshy, Pavel approves or makes counterparts in Tripo and decides how we work until the end. Weapons as separate models. Hero and boss on one skeleton.

## 5. Hour-by-hour plan

Hard points: **K1** playable version, **K2** test build, **K3** feature freeze, **K4** video, **K5** submission.

**Schedule change (the author, Sat 14 Nov 18:30):** the Sat 22:00 feature freeze (K3) is lifted, work goes on late
into Saturday night. New rule: every milestone ends with a green Shipping build pushed to itch, so there is always
something that could be submitted. New features stop **Sun 12:00 PC clock** (the video capture needs the final look).
Video Sun 18:00 and submit Sun 20:00 (never later than 21:59) stay.

### Thu 12.11: fight prototype with beans

| Time | Claude | People |
|---|---|---|
| 13:30–14:00 | New session. Session entry in `sessions.json`, `DEVLOG.md`, tickets, butler. Hero, boss and arena concepts started in the background | Pavel: draft itch page, address to the chat |
| 14:00–16:30 | Block A: arena, the bean walks, aim, three cameras on F1/F2/F3, a ball, a dummy boss with health | Pavel looks at the concepts when he can |
| 16:30–19:00 | Block B: Katana, Shuriken, Dash as components with an owner. Boss: chase, volley and area strike | |
| 19:00–21:30 | Block C: thefts at thresholds with a choice card, the boss uses stolen mechanics, Retry from the last theft, victory and defeat as text | Kirill: music mood, a sketch of the theft card |
| 21:30–23:00 | A bot run, packaging, upload to the draft itch page, one clip per camera | |
| **23:00, K1** | **Playable version:** the fight from the start to victory or death in a packaged build on itch | Pavel plays on Windows and picks a camera by 07:00 |

If time remains: Parry and Ultimate from block D.

### Fri 13.11: the full fight and a test build

| Time | Claude | People |
|---|---|---|
| 07:00–12:00 | Block D: the chosen camera, the others off. Parry, Ultimate, five mechanics and thresholds 80/60/40/20, the finishing blow with the comeback, titles | |
| 12:00–17:00 | Block E: cosmetic layers on placeholders (effects, grid, T-pose, bean, music tracks). First models in Meshy: hero and boss, check the chain model, rig, animations, import | Pavel approves the concepts |
| **18:00, K2** | **Day 2 test build:** Shipping on the draft itch page | Pavel and Kirill download and play |
| 19:00–24:00 | Feel fixes from live notes. Import of the first sounds | Pavel at home: approve the models or redo them in Tripo, decide on the art pipeline. Kirill: tracks v1 |

### Sat 14.11: art, fight feel, balance

| Time | Claude | People |
|---|---|---|
| 09:00–13:00 | Block F: art into the game. Hero and boss with animations, neon arena, effects | Pavel: models, two props in Rodin, intro decision |
| 13:00–18:00 | Block G: music and sounds, readability of the boss's attacks, intro (if decided) | Kirill: final tracks, sounds, interface |
| 18:00–22:00 | Balance: a bot runs all theft orders, people play | Everyone plays. Evgeny: bug list |
| ~~22:00, K3~~ | Lifted (Sat 18:30): the fight block (T-055, T-038, T-056, T-057, T-058), then the Meshy models T-025/T-026, T-030, T-039 into the night. Each milestone ends with a green Shipping build on itch | Pavel plays builds between tickets |

**Sun 12:00, K3 (moved):** new features stop. After it only bugs, balance, polish, English text.

### Sun 15.11: polish and submission

| Time | Claude | People |
|---|---|---|
| 09:00–12:00 | Last features until 12:00 (K3 moved here), bugs, performance, English proofreading. Raw 1080p clips right after 12:00 on the final look | Playtest |
| 12:00–15:00 | itch page, `THIRD_PARTY.md`, tool list, final dev log, NVIDIA answers (if DLSS made it) | Pavel edits the video |
| 15:00 | Release candidate: `TeamName_GameName.zip`, size check | Kirill or Evgeny run it on a clean machine |
| **18:00, K4** | **Video ready** and uploaded, link in hand | Pavel |
| 19:00 | Final build on the itch page, form filled in | Pavel checks the form |
| **20:00, K5** | **Submission** (target). Hard limit 21:59 | Pavel presses the button |

After submission we touch nothing: a second submission is a disqualification (`JAM_BRIEF.md`, section 3).

## 6. How the agent works

- The prototype is made by one session without subagents, ticket by ticket through tests. Later, lead subagents are allowed, but before each wave a forecast (agents, tokens, share of the limit, a cheap option) and Pavel's "yes".
- After each block: a commit, a 30–60 second clip, three lines to the chat and to `DEVLOG.md`.
- Questions that need Pavel's eyes go into `QUESTIONS.md` and do not stop the work: the agent takes the default option and moves on.
- Packaging every evening. A warning before launching a game window.
- Session logs are exported to `_AgentLogs/` on every commit. The session talks in English. Secrets are sent to the chat as a `NAME_KEY=value` line.

## 7. Prizes and perks

- We go for Best Overall UE, Best Team, Glass Box. Render Royalty as a hope.
- Hyper3D: a bonus for winners of any category. Two props from Rodin in the game and Rodin in the Tools used field.
- RTX Award: under "if time allows". The DLSS plugin is enough, but the country list is not checked.
- Perks: fal, Meshy, Tripo, Hyper3D, Anthropic $25. WeLoveIndies only if sounds run short (non-commercial licence, goes into `THIRD_PARTY.md`).

## 8. Risks

| Risk | What we do |
|---|---|
| The whole game is one fight, no backup ending | A bean prototype today, a bot player for balance, people play from Thursday night |
| Three cameras smear the day | The simulation is shared, cameras are a thin layer. Choice by Fri 07:00, the two extra ones are switched off |
| The bean cannot beat the boss with the full set | The bot runs all theft orders, damage and health in a data table, Retry from the last theft |
| The model, rig, animation chain will not work | We check it on the hero on Friday afternoon. Backup: rig in Tripo |
| Pavel without the home PC until Friday evening | Builds on the draft itch page, clips after each block |
| Packaging breaks at the end | Shipping is built every evening. The empty project is already packaged |
| Claude's limits run out mid-block | Blocks no longer than five hours, heavy waves only by forecast |
| Russian text or sound in the game | All strings in one place, proofreading on Sunday morning |

## 9. Checklist

Done:
- [x] Commit `pre-jam: tooling and plan`, the empty Shipping build packaged and launched.
- [x] fal, Meshy and itch keys in `.env`. Meshy and itch checked with a request, fal not checked.
- [x] The log export strips keys. The night planning session is not in the log, the development log starts with the first jam session.

Pavel:
- [ ] Create a draft page on itch and send the address.
- [ ] A Windows PC for tonight: play build K1 and pick a camera by Fri 07:00.
- [ ] Activate Hyper3D with the code from the email by 16.11.
- [ ] Check the RTX Award country list in the official rules.
- [ ] Team name by Sunday.
- [ ] Cancel the Meshy subscription before the free month ends.

Kirill:
- [ ] Register on itch, join the jam, give the email of the Epic account.
- [ ] A Google Drive folder shared with Pavel.

Evgeny:
- [ ] Answer by Sat 14.11, 12:00: in or out. If in, name and email of the Epic account.

Claude in the first hour of the session:
- [ ] Check the name NEON RONIN is free.
- [ ] Install butler and upload an empty build to the draft.
- [ ] Check reading files from the Google Drive folder on one WAV.
