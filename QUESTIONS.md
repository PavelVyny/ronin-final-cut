# Questions for the author

Things that need Pavlo's eyes. The work does not wait: each question has the default the agent already took.

## Open

### Q2. Which hero and which boss? (Thu 13:39, answered Sat 13:45: hero A, boss r4_boss_c2c_final)

- Files `Art/Concepts/r1_*.png`, sent to the chat. Hero A (lean, black with cyan and magenta piping) or hero B
  (kasa hat, indigo coat). Boss A (charcoal suit, kabuto, gold armour) or boss B (white suit, gold, sunglasses).
- **Default:** hero A and boss A: they match the lineup image and both arena paintings. No model is ordered before
  your answer; hero animations wait for the camera choice anyway.
- Sat 11:55: all sheets on one page for the pick: https://claude.ai/artifact/JPCnbPCnQkjLDX57abegRW (private). The
  author picks there or suggests new options (a round 2).
- Sat about 12:00, new brief from the author: a heavy fat-cat Publisher, slow and clumsy, each blow crushing, our own design.
  Round 2 (`Art/Concepts/round2_jobs.json`, 0.30 USD): C1 pinstripe fat cat (flagged: close to a well-known comic
  crime boss), C2 sumo executive, C3 Boss A made heavy with a round lacquered belly plate.
- **Boss answer (Sat about 12:08, voice to text):** "C3, but without the golden bell on his belly." Read as: C3 without
  the gold-banded belly plate. Variant `r2_boss_c3b` (0.10 USD) generated for the author to confirm. **Hero still
  open:** default hero A.
- **Skeleton answer:** "the way you think fits best, polish later in Blender with the MCP." Taken: one humanoid
  skeleton for both (one rig pipeline), the boss gets his own few heavy clips (waddle, two-hand swing, slam, hit
  react) instead of the hero's.
- **Numbers answer:** the heavy profile (ticket T-041): slower, bigger, longer wind-ups, harder hits, bot table rerun.
- **Sat about 13:30, the author changed his pick:** "boss C2 I liked the most. Just remove the golden belt and keep the
  tie and golden gloves as is", and asked for more comic, sarcastic versions ("how a game developer sees the
  publisher with no polite filters"). Round 3 (`round3_jobs.json`, 0.40 USD): `r3_boss_c2b` (the edit as asked),
  `c2c` (dollar eyes, gold tooth, cash, a phone with a rising chart), `c2d` (dollar-print suit, coin crown, monocle),
  `c2e` (headset, dollar shades, blank sticky notes, a red rejection stamp holstered).
- **Final answer (Sat about 13:45):** "everything is good about C2c, remove only the graph from the tie": the boss is
  `Art/Concepts/r4_boss_c2c_final.png` (round 4, 0.10 USD). **Hero: A** (confirmed). The boss model is made by the
  next session through the Meshy API (`MESHY_API_KEY` in `.env`), the author judges it. New VFX asked for: the
  Publisher bleeds paper dollars when hit (T-042).

### Q3. What happens when the steal timer runs out? (Thu 13:40)

- The plan says the fight slows for 3 seconds and the player picks. It does not say what happens without a pick.
- **Default:** the Publisher takes the left option.
- Alternative: he takes a random one, or the slowdown lasts until the player picks.

### Q5. The Google Drive folder from Kirill

- Needed to check reading one WAV. Not blocking: the stubs play under his names (Q8).
- Sat 11:55: asked again in chat; the author pastes the link when he has it.

## Answered

### Q1. The name (Thu 13:38, answered Sat 14 Nov 11:50 in chat)

- NEON RONIN is Steam app 1929540 by TYPE-III. **Answer: RONIN: FINAL CUT.**
- Checked Sat 11:55 (web search): no game with the exact name "Ronin: Final Cut". Near names exist: RONIN (2015,
  Devolver Digital, Steam app 274230, born as an itch jam entry), "final cut" by xanderday on itch (a 2D prototype),
  "The Last Oricru: Final Cut" on Steam. The rename is its own ticket (T-040).

### Q7. The cut line (Sat 00:49, answered Sat 14 Nov 11:50 in chat)

- **Answer: keep everything**, then confirmed: build all three items the plan proposed to cut. The Publisher uses the
  Parry and the Ultimate he stole (T-038), a short intro (T-030), DLSS (T-039, last, the first to go if the clock
  runs out).

### Q8. The sound list for Kirill (Sat 02:55, answered Sat 14 Nov 11:50 in chat)

- **Answer: approve the 13 names** of `Tickets/T-028_sound_effects.md`: `sfx_katana_slash_01`,
  `sfx_shuriken_throw_01`, `sfx_explosion_01`, `sfx_slam_01`, `sfx_dash_01`, `sfx_parry_01`, `sfx_ultimate_01`,
  `sfx_boss_hit_01`, `sfx_hero_hurt_01`, `sfx_steal_card_01`, `sfx_steal_01`, `sfx_everything_back_01`,
  `sfx_final_blow_01` (WAV, 48 kHz). The Ultimate-ready fix (T-037) added a 14th cue, `sfx_ultimate_ready_01` (a
  short rising chime when the hero's meter fills): it goes on Kirill's list too. The cash heal (T-049, Sat 15:23)
  added a 15th, `sfx_heal_cash_01` (a cash register "ka-ching" when the hero picks up the Publisher's cash stack).

### Q9. A longer fight (Sat 02:55, answered Sat 14 Nov 11:50 in chat)

- **Answer: keep 4800** after three fights on the session-3 build; revisited after the playtest fixes.

### Q10. Team name for the submission zip (answered Sat 14 Nov 11:50 in chat)

- **Answer: Pavyny.** The zip is `Pavyny_<GameName>.zip`.

### Q4. The itch draft page address

- **Answer (Sat 14 Nov, in chat):** https://pavyny.itch.io/neon-ronin, channel `windows`. butler v15.32.0 installed in
  `~/bin`. The first push was blocked by an unverified itch email (Q4b); the author verified it on Sat 14 Nov 02:15
  and the block D build was pushed (`Tools/Build/itch_push.sh`).

### Q6. Which camera? Answer by Friday 07:00 (Thu 14:10)

- Clip `Clips/block_a.mp4`, sent to the chat: 8 seconds on each camera with a bot playing. In the build: F1, F2, F3.
- The clip shows the look only. The feel of the aim (cursor against mouse look) needs your hands on the 23:00 build.
- Thu 16:05: one 50 second clip per camera, sent to the chat: `Clips/k1_cam0_topdown.mp4`,
  `Clips/k1_cam1_shoulder.mp4`, `Clips/k1_cam2_firstperson.mp4`.
- **Default:** top-down 3/4. The cursor aim is the most exact, the boss's area attacks read best from above, and the
  art is the cheapest (no first-person arms, no close-up of the hero's back).
- **Answer (Fri 13 Nov, in chat):** camera 0, top-down. The other two are switched off in T-018.
