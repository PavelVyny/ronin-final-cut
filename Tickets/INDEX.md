# Tickets

One file per ticket. Status: `todo`, `doing`, `done`, `cut`. A ticket is done when its `Jam.` tests are green in
`build_and_test.sh` and the change is committed.

## Architecture in five lines (decided at the start of block A)

- The fight is a **pure simulation**: `FJamFight` (plain data: two fighters, projectiles, hazards, the mechanic
  ledger, the steal state) advanced by `JamFightRules::Step` at a fixed 60 Hz. No world, no actors, flat arena in 2D.
- Everything a fighter does comes in as an **intent** (`FJamIntent`: move, aim, buttons). The player controller, the
  boss brain and the hero bot all write intents, so the hero and the boss run the same mechanic code.
- A **mechanic** is a sim-side component with an owner; a steal is a change of owner in the ledger.
- Folders: `Core` (types, balance, ini loading), `Sim` (rules and the subsystem that hosts the fight), `Game` (glue:
  game mode, controller, demo run), `View` (arena, fighters, projectiles, cameras), `UI`. `Core` and `Sim` never
  include `View`, `UI`, `Audio`. The game mode owns the frame order: input, simulation, views, camera.
- Balance numbers live in one place: `[JamBalance]` in `Config/DefaultGame.ini`.

## Thursday: the prototype on beans

| Ticket | Block | Title | Status |
|---|---|---|---|
| [T-001](T-001_skeleton_balance_fixed_step.md) | A | Skeleton: folders, game mode, arena map, balance data, fixed-step sim driver | done |
| [T-002](T-002_bean_moves.md) | A | The bean moves in the arena | done |
| [T-003](T-003_aim.md) | A | Aim abstraction | done |
| [T-004](T-004_three_cameras.md) | A | Three cameras on F1, F2, F3 | done |
| [T-005](T-005_ball_shot.md) | A | The weak ball shot and projectiles | done |
| [T-006](T-006_boss_dummy_health.md) | A | Boss dummy with health, debug HUD | done |
| [T-007](T-007_mechanic_ledger.md) | B | Mechanics with an owner, the ball fallback | done |
| [T-008](T-008_katana.md) | B | Katana | done |
| [T-009](T-009_shuriken.md) | B | Shuriken | done |
| [T-010](T-010_dash.md) | B | Dash | done |
| [T-011](T-011_boss_brain.md) | B | Boss brain: chase, memo volley, telegraphed slam | done |
| [T-012](T-012_steal_thresholds.md) | C | Steals at health thresholds | done |
| [T-013](T-013_choice_card.md) | C | The choice card and the slowdown | done |
| [T-014](T-014_boss_uses_stolen.md) | C | The boss uses what he stole | done |
| [T-015](T-015_retry_win_loss.md) | C | Death, retry from the last steal, win and loss titles | done |
| [T-016](T-016_hero_bot.md) | K1 | Hero bot and full-fight tests | done |
| [T-033](T-033_boss_answers_kiting.md) | K1 | The Publisher answers kiting: a volley that leads its target | done |
| [T-017](T-017_k1_build.md) | K1 | Checkpoint K1: Shipping build on itch, a clip per camera | doing (built, waits for the itch address) |

## Friday to Sunday: stubs, detailed on Friday morning

| Ticket | Block | One line |
|---|---|---|
| [T-018](T-018_top_down_only.md) | D | Keep the chosen camera (0, top-down, chosen Fri 13 Nov), switch the other two off (done) |
| [T-019](T-019_parry.md) | D | Parry: timed deflect of a projectile back at its owner (done; the boss only takes it until the cut answer) |
| [T-020](T-020_ultimate.md) | D | Ultimate: a meter filled by hits, slowdown and a slash across the screen (done; the boss only takes it until the cut answer) |
| [T-021](T-021_five_mechanics_four_cuts.md) | D | Five mechanics, thresholds 80/60/40 with a pair and the FINAL NOTE at 20 (done) |
| [T-022](T-022_finisher_with_the_ultimate.md) | D | The finisher: boss stunned at 0, everything returns at once, the Ultimate lands the final blow, both titles (done) |
| [T-023](T-023_cosmetic_layers.md) | E | Cosmetic layer switches on stand-ins: VFX off, textures to a grid, animations to a T-pose, hero to a bean, HUD cut (done) |
| [T-024](T-024_music_layers.md) | E | Music stems as layers that drop with each steal, the PoC metronome loop (done, stub stems) |
| T-025 | E | Art pipeline check on the hero: Meshy model, rig, animations, import |
| T-026 | F | Hero and boss models with animations in the game, one skeleton |
| [T-027](T-027_neon_arena.md) | F | The neon arena, VFX for every mechanic and boss attack (done on stand-ins; real VFX wait for the art) |
| [T-028](T-028_sound_effects.md) | G | Music and sounds from Kirill's Drive folder, UI from his Figma designs (doing: stub sound effects in, his files wait) |
| [T-029](T-029_readability.md) | G | Readable boss attacks: hit feedback, camera shake, the explosive in his hands (done) |
| [T-030](T-030_intro.md) | G | Intro: the Publisher's kung-fu show-off under a title card, skippable (done) |
| [T-031](T-031_balance.md) | G | Balance: the bot runs every steal order, numbers tuned in the balance section (done for the bot; a person's fight is not measured) |
| T-032 | Sun | Submission: itch page, `THIRD_PARTY.md`, tool list, dev log, release candidate zip |

## Saturday session 4: the author's playtest and the cut line answer

| Ticket | One line |
|---|---|
| [T-034](T-034_instant_aim.md) | The hero turns to the mouse on every frame, a neon hardware pointer (done) |
| [T-035](T-035_bare_swing_shield_parry.md) | After the effects cut melee still shows a bare grey swing; Parry is a standing shield (done) |
| [T-036](T-036_parry_prompt.md) | "Q PARRY" over the hero when a shot is about to hit, until three deflects (done) |
| [T-037](T-037_ultimate_called_out.md) | The Ultimate meter as a bar, a pulsing "E ULTIMATE READY" banner and a ready cue (done) |
| [T-041](T-041_heavy_publisher.md) | The heavy Publisher (boss C3b): slower, bigger, longer wind-ups, harder hits, bot table rerun (done) |
| [T-038](T-038_stolen_parry_and_ultimate.md) | The Publisher uses the Parry and the Ultimate he stole: a shield stance and a timed line |
| T-030 | A short intro before the fight (Q7 answer) |
| T-039 | DLSS (Q7 answer; last, the first to go) |
| [T-040](T-040_rename.md) | Rename to RONIN: FINAL CUT (Q1 answer; done: win title, window title) |
| [T-042](T-042_dollar_bleed.md) | The Publisher bleeds paper dollars when hit (done) |
| [T-043](T-043_shorter_final_stretch.md) | Playtest: 4000 health, a short ball-only stretch, 7 s on the steal card (done) |
| [T-044](T-044_parry_on_strike.md) | Playtest: Parry on LMB, triggered by an enemy shot in reach (done) |
| [T-045](T-045_boss_marker.md) | Playtest: an arrow at the screen edge to the Publisher when he is off screen (done) |
| [T-046](T-046_ultimate_lane.md) | Playtest: the Ultimate shows where and when it lands (done) |
| [T-047](T-047_arrow_gap.md) | Playtest 2: the boss arrow stops under his health bar (done) |
| [T-048](T-048_checkpoint_health.md) | Playtest 2: a retry brings back the health of the cut, 50% if it was 20% or less (done) |
| [T-049](T-049_cash_heal.md) | Playtest 2: the Publisher drops a cash-stack heal, with a ka-ching (done) |
| [T-050](T-050_slam_pattern.md) | The slam grows with each cut: 1, 3, then 5 circles landing 0.2 s apart, always a spot to walk to (done) |
| [T-051](T-051_katana_untouchable.md) | Playtest 3: the Katana is never on a choice card, only the FINAL NOTE takes it (done) |
| [T-052](T-052_slam_spread.md) | Playtest 3: the five-circle slam spreads out, a 200 cm gap between any two circles (done) |
| [T-053](T-053_health_bar_chunk.md) | Playtest 3: delayed damage chunk and heal preview on both health bars (done) |
| [T-054](T-054_money_spills.md) | Playtest 3: dollars and the cash stack spill out of the Publisher's body and land clear of him (done) |

## Saturday evening: the fight block (the author, Sat 14 Nov 18:30; the freeze is lifted)

| Ticket | One line |
|---|---|
| [T-055](T-055_stolen_at_full_strength.md) | What he stole hits at full strength, a shorter pause (0.9 / x1 / 0.8 / 0.7) |
| [T-038](T-038_stolen_parry_and_ultimate.md) | His stolen Parry is a shield stance, his stolen Ultimate a timed locked line |
| [T-056](T-056_signature_combos.md) | A signature combo per stolen mechanic |
| [T-057](T-057_parry_buffer.md) | An early parry click waits for the shot (0.3 s look-ahead) |
| [T-058](T-058_ultimate_turns.md) | The Ultimate turns toward the cursor during its wind-up, 120 deg/s |

## Saturday night: feedback on session-5i, then the models (the author, Sat 14 Nov 19:50)

| Ticket | One line |
|---|---|
| [T-059](T-059_parry_rearm.md) | A good parry is ready again at once: spamming LMB sends back every shot of a burst (done) |
| [T-060](T-060_his_shurikens_plain.md) | His stolen shurikens never explode, the parry catches them 30 cm further out (done) |
| [T-061](T-061_models_in_game.md) | The 3D hero and Publisher replace the stand-ins, clips timed to the simulation (done) |
| [T-062](T-062_props_in_game.md) | The shuriken, cash stack, memo and dollar cards replace the greybox shapes (done) |
| [T-063](T-063_hero_cloth_patches.md) | The glow patches on the hero's black cloth painted over, piping and mask kept (done, for review) |
| [T-064](T-064_hit_lean_afterimages.md) | A hit is an additive upper-body lean, the hero leaves four neon afterimages (done) |

## Sunday night polish (the author, Sun 15 Nov 02:40; plan in PLAN_polish_night.md)

| Ticket | One line |
|---|---|
| [T-065](T-065_hands_and_blade.md) | The katana held in the fist in every pose, throws from the left hand, shots leave the hand (done) |
| [T-066](T-066_smooth_motion.md) | Actions crossfade, the Publisher turns at a rate, backpedalling, a dash leaves afterimages (done) |
| [T-067](T-067_three_tiers.md) | Three tiers: the 3D hero and full art, then the shape ronin, then greybox (done) |
| [T-068](T-068_crack_transition.md) | A change of look tears the screen: the old frame splits along a neon crack (done) |
| [T-069](T-069_rich_effects.md) | Niagara sparks, dust, cash, embers and trails for the full-art tier (done) |

## Sunday feedback on session-5m (the author, Sun 15 Nov)

| Ticket | One line |
|---|---|
| [T-070](T-070_his_shurikens_slower.md) | His stolen shurikens and his shuriken burst come 30% slower (done) |
| [T-071](T-071_slow_fault_crack.md) | The crack runs down a jagged fault line, holds, and the halves slide off over 2.1 s (done) |
| [T-072](T-072_charge_aura.md) | The Ultimate charge is a Niagara aura on the hero in full art, a floor glow one tier down, bars in greybox; the dash loses its bar (done) |

## Sunday afternoon, parallel tracks (the author, Sun 15 Nov)

| Ticket | One line |
|---|---|
| [T-073](T-073_meter_bar_back.md) | The Ultimate meter bar is back in every tier; the aura shows only when full (done) |
| [T-074](T-074_hero_shuriken_rate.md) | The hero throws 30% more shurikens for the same damage a second; his stolen copy pinned (done) |
| [T-075](T-075_full_art_without_discs.md) | The full-art tier leaves hits, slams, blasts, deflects and heals to Niagara (done) |
| [T-076](T-076_niagara_rework.md) | Hits flash and light the floor, slams ring out, cash sparkles (done) |
| [T-077](T-077_tier_light_and_post.md) | Lumen, neon lights and a grade for the full art; flat, then bare, below (done) |
| [T-078](T-078_legs_follow_the_move.md) | The hero's legs run where he moves, not where he faces: four Meshy diagonals, hip twist, rate and lean (done) |
| [T-079](T-079_publisher_feet.md) | The Publisher's feet keep up with him (done) |
| [T-080](T-080_backpedal_reversed.md) | A backpedal plays the plain run backwards (done) |
| [T-081](T-081_rooftop_shrine.md) | The full-art arena is a rooftop shrine in the rain (Meshy props, PBRgen slate, rain and splashes); the first cut leaves the bare shrine, the second the grid (done) |
| [T-082](T-082_light_the_publisher.md) | A key light on the Publisher, a brighter night, no glow pools under the fighters (done) |
| [T-083](T-083_boss_bar_after_intro.md) | The Publisher's health bar fades in when the fight starts (done) |
| [T-084](T-084_throw_while_backpedalling.md) | A throw while backpedalling keeps the upper body (done) |
| [T-085](T-085_crack_sound_once.md) | The crack's sound plays once, short and quiet; a thinner line (done) |
| [T-086](T-086_throw_mirror_fold.md) | The throw no longer folds the torso: the mirror keeps each bone's roll (done) |
| [T-087](T-087_spot_operator.md) | The Publisher's spot trails him like a hand works it (done) |
| [T-088](T-088_shuriken_floor_light.md) | Flying shurikens light the floor under them (done) |
| [T-089](T-089_hero_draw_intro.md) | The hero draws his blade as the intro's camera comes to him (done) |
| [T-090](T-090_publisher_faces_hero.md) | The Publisher's show-off ends facing the hero (done) |
| [T-091](T-091_steal_comments.md) | The Publisher comments on the move he stole (done) |
| [T-092](T-092_main_menu.md) | The main menu: new game, settings, exit, over the two fighters (done) |
| [T-093](T-093_opening_comic.md) | The opening comic: the call that starts the fight (done) |
| [T-094](T-094_menu_comic_polish.md) | Menu, comic and card polish after the author's first look (done) |
| [T-095](T-095_session5s_fixes.md) | After session-5s: comic sounds, banter, the hero's later tier, the rune telegraph (done) |
| [T-096](T-096_how_to_play.md) | HOW TO PLAY, the itch page images, the last look fixes (done) |
| [T-097](T-097_final_playthrough_fixes.md) | The author's final play-through: comic timing and glass, the intro's turn and theme (done) |
| [T-098](T-098_window_fits_desktop.md) | A window never larger than the desktop (done) |
| [T-099](T-099_rodin_balance_menu_crisp_ui.md) | Rodin props, a kinder fight after the first cut, the menu's first frame, crisp UI (done) |
| [T-100](T-100_hero_spot_easier_fight.md) | The hero's spotlight, an easier fight, the old cash stack, the comic's glass (done) |
| [T-101](T-101_difficulty_levels.md) | Difficulty levels: EASY, NORMAL, HARD on the main menu (done) |
| [T-102](T-102_sharp_first_launch_dev_build.md) | A sharp first launch, a Development build without developer tools (done) |
| [T-103](T-103_slate_relief.md) | The slate floor's relief: normal strength, cavities, parallax, the menu's rim light (done) |
| [T-104](T-104_neon_bars.md) | Neon health and Ultimate bars in the full art, plain ones after the first cut (done) |
