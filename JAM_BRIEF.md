# JAM_BRIEF: 2026 Epic AI MegaJam

> Translated from Russian on 2026-11-17, after the jam. The original Russian text is in the first commit of this
> repository. Nothing was added or removed: it is the brief as written before the start.

Compiled 2026-11-11 23:55 Oslo time (6 hours before the start). All times are Europe/Oslo (CET, UTC+1) unless stated otherwise.

Sources and what was checked in each:
- **[PDF]** Official rules, Russian edition of 26.10.2026, all 18 pages read.
- **[itch]** https://itch.io/jam/2026-epic-ai-megajam, read through WebFetch (it paraphrases the page, so exact wording from the page should be checked by eye where marked).
- **[stream]** Kickoff recording (49 min), YouTube auto-captions, read in full. The theme is quoted from them.
- **[forum 5.9]** https://forums.unrealengine.com/t/unreal-engine-5-9-megajam-preview/2852840, first page of the thread.
- **[X]** The @UnrealEngine post did not open (HTTP 402). The theme wording comes from the stream, where the host read it out from the graphic twice.

---

## 1. Theme

**DIRECTOR'S CUT**

Verbatim from the stream (25:00 and 43:00, read out from the graphic):

> Your vision, your specific edits, and ultimately your final cut. Your director's cut. Every developer has that one feature that didn't quite make it, the mechanic scoped out while sleep-deprived at 2 am, or that idea you've had shelved and never quite found a place for yet. Well, now is your chance to finally build it all in this year's Epic MegaJam. If you're utilizing AI agents in your workflows, you're directing those too. Set the vision, delegate the tasks, and decide what goes in and what doesn't. Aim for ambition and direct your path accordingly.

How it was explained:
- Host (34:00): the theme is "both specific and vague for a reason", they want to see our interpretation of what a director's cut is.
- Host (44:00): it has a double meaning. First: you direct the agents. Second: what you can do with agents in 4 days that was out of reach before. "There's room for interpretation, go wild with it". He likes entries best where the theme is cleverly woven into the game design.
- Guest from Epic (Adrian, product technical director): "I have some themes that I let die", so the literal reading "take an idea off the shelf" is expected too.
- **No** concrete game examples were given on the stream.

Judging criterion for the theme [itch, stream 21:00]: "a fresh, clever take on the theme".

My assessment (not from the sources): the reading "this is my long-held idea" works only in the description and the dev log, a judge will not see it in the game itself. Most teams will take it, so it does not reach "fresh, clever". Stronger are concepts where editing, cut content, revisions or the "final version" are the mechanic.

## 2. Timeline (Europe/Oslo)

| When | What |
|---|---|
| Wed 11.11, 20:00 | Kickoff and theme (done) |
| **Thu 12.11, 06:00** | **Start.** Before it only tools, agents, MCP, prompts, scripts |
| Thu–Sun, ~20:00 | Daily streams. The page says 11 AM PST, but the host said the schedule moved to fit the guests, and the graphic did not make it into the captions. **Exact time not verified**, check the channel |
| Fri 13.11 | Day 2: a test packaged build (Epic's advice) |
| Sat–Sun 14–15.11, 18:00–04:00 | In-person venue in San Francisco (not needed for us, online entries are judged the same) |
| **Sun 15.11, 20:59** | **Our internal submission deadline** (6 hours early) |
| **Mon 16.11, 02:59** | **Official deadline** (15.11 20:59 ET). Late = disqualification |
| by 29.01.2027 | Winners announced. 72 hours to answer the email, otherwise the prize goes to someone else [PDF §14] |

Calculation check: itch shows the submission window 2026-11-12 05:00 → 2026-11-16 01:59 with no time zone, that is UTC, plus an hour gives Oslo. Matches your numbers.

Note: the clock on this PC is currently UTC+2 (GTB). If you are not in Norway, by local time the start is at 07:00 and the deadline at 03:59.

Deadlines of perks that expire:
- Cursor + Grok: claim by 15.11, valid 7 days.
- Hyper3D Rodin: activate by 16.11, valid until 20.11.
- Tripo: by 16.11 (already activated for us), credits until 17.11.
- Meshy: by 17.11, needs a card.
- PBRgen: valid only 7–15.11.
- Diversion: by 22.11. Anthropic API $25: 12–27.11. fal $30: by 30.11.

## 3. Submission checklist (UE)

Game:
- [ ] A packaged build, not a project. Windows 10/11, macOS, Android or iOS. Safe choice: Windows.
- [ ] Runs without installing extra software [PDF 7(f)].
- [ ] Archive `TeamName_GameName.zip`, under 1 GB (larger only through butler).
- [ ] Uploaded to **our** itch.io account, then submitted to the jam. One entry per team, a second submission = disqualification [PDF §2].
- [ ] Everything in English: in-game text, voice, video, description [PDF 7(i)].

Video:
- [ ] 30–60 seconds, at least 1920x1080, gameplay footage, as a link.
- [ ] Gameplay in the first 5 seconds (Epic's advice).

Data:
- [ ] Team name and game name.
- [ ] Full names and emails of all three, the ones tied to Epic accounts. One marked as Team Owner.
- [ ] List of third-party assets (Fab, WeLoveIndies etc.). AI-generated content does not count as third-party [PDF 15(c)].
- [ ] Declare everything made before the start: prompts, scripts, configs [itch FAQ].
- [ ] Dev log as a link (required for Glass Box).
- [ ] List of tools (needed for partner prizes: Rodin, NVIDIA).
- [ ] Answers to the NVIDIA questions, if we go for the RTX Award.

## 4. Restrictions

Before the start [PDF 7(h)]:
- Allowed: AI tools, agent configurations, MCP, prompt libraries, scripts. Epic may request these materials or their change history in VCS.
- Not allowed: gameplay, levels, art. All of that only within the jam. Exception: third-party assets.

Content:
- Not a rework of Epic tutorials (renaming and recolouring does not count) [7(d)].
- Gameplay beyond Epic's starter templates [7(e)].
- Teen rating (13+): no nudity, animal cruelty, drugs, slurs.
- Other people's IP is forbidden. All trademarks and logos are forbidden except the Unreal Engine logo per the brand guide [15(d)].
- No criticism of people and recognisable products, Epic's competitors included [§8]. Satire of real games and studios is off limits.

Participants [PDF §5]:
- 18+, legal resident of an eligible jurisdiction, Epic account in your own name.
- Excluded jurisdictions: Iran, Iraq, North Korea, Somalia, Sudan, Syria, Crimea, Donetsk, Luhansk, **Italy**. Russia, Turkey and Brazil were removed from the list on 26.10 (Brazilian residents without cash prizes).
- Team of 1–5 people, a person is in one team only.
- All money and prizes go to the Team Owner (Hyperwallet, W-8BEN form), who splits them within the team [§13].

Engine: UE or UEFN, not both. Any UE5 version, 5.9 is not required [stream 07:50].

## 5. Judging

Score 1–5 on each of seven criteria [PDF §9, itch, stream]:

1. **Ambition and innovation**: what you usually cannot pull off in 4 days. A new mechanic, a bold style, a bigger scope.
2. **Theme**: a fresh, clever take.
3. **Fun factor**: fun and pleasant to play.
4. **Gameplay**: coherent, original mechanics that feel good.
5. **Visuals**: a strong unified style, photoreal or stylised.
6. **Level design and progression**: spaces that support the gameplay and guide the player.
7. **Audio**: sound and music that enhance the experience.

Important:
- Innovation is judged in the game itself. Using AI gives no points, information about it is only context [PDF §9].
- On a tie, a recount with Fun factor doubled [PDF §9]. Fun matters more than anything else.
- One entry gets at most one main prize and one special prize [PDF §13]. A contender wins only one special category [PDF §10].
- The winner is decided by the total score: all seven criteria weigh the same. Sound and level design give as much as gameplay. A game without sound loses 1/7 of the score.

## 6. Categories

Main: Best Overall UE ($7500 / $5000 / $2500), UEFN, Student (the same).

Grants to continue (not a prize, at Epic's discretion): $50k / $25k / $10k plus Fab credits $1000 / $500 / $250.

What to aim for:

| Category | Prize | What it takes | Verdict |
|---|---|---|---|
| **Best Overall UE** | up to $7500 + grant | Total score | Main goal |
| **Best Team** | $5000, the biggest modifier | Team of 2+ | We qualify automatically |
| **Glass Box** | $2500 + ROG Ally each | Dev log submitted on time | Cheap for us: agent session logs and the docs-sync habit already exist. Keep it from the first minute |
| **Render Royalty** | $2500 + partner prizes | Best visuals, stylised counts | Realistic with a unified style (Tripo + Blender pipeline exists) |
| **NVIDIA RTX Award** | RTX 5090 | UE, at least one NvRTX feature, DLSS 4.5 or NVIDIA ACE, answers to questions at submission | The DLSS plugin may be a cheap ticket. Needs an RTX card and a country check |
| Best Student | up to $7500 | All three are university students | Ask at the grill |
| Pocket Powerhouse | $2500 | Mobile game | Not advised: mobile packaging is a separate risk |
| Solo Driver, Walkie-Talkie | | Solo, UEFN | Not ours |

Partner prizes for winners of any category: Hyper3D (a year of Rodin Business each plus a share of a **$10,000 cash pool** among winning teams that used Rodin and listed it in Tools used), Tripo, Meshy.

## 7. Perks: what to claim

Claim for sure:
- **Tripo**: 3000 credits, already activated. Main 3D generator, the pipeline is tuned.
- **Hyper3D Rodin**: 208 credits. Generate at least a couple of assets in it that make it into the game and list it in Tools used: that is the ticket to the $10k cash pool.
- **WeLoveIndies**: sound and music. Non-commercial licence, a commercial release would need a purchase. Goes into the third-party asset list.
- **Anthropic**: $25 API (useful for generation scripts).
- **fal**: $30 (images, video, sound through the API).

Depending on the situation:
- **Meshy**: 1000 credits, needs a card. A reserve in case Tripo fails.
- **PBRgen**: PBR materials, valid only until 15.11.
- **Diversion**: cloud VCS. You already have dv installed. Decide git or dv before the start, the point is one system for all three.
- **Cursor + Grok $50**: if someone on the team works in Cursor.
- **Reallusion iClone, Rokoko**: only if the concept has character animation and someone to do it. Learning a new tool at a jam is not worth it.

## 8. UE 5.9 MegaJam Preview: do we need it

What it gives [forum 5.9, MCP documentation]:
- MCP starts with the editor, the connection is more reliable. New toolsets: rendering, VCS, profiling, debugging, gameplay testing, geometry, sound, UI, level creation, Fab (the agent finds and installs assets).
- Unreal Assistant: an editor panel for Claude Code, Codex, Cursor.
- More Python API: sound, MetaSounds, Niagara, materials, PCG, UI.
- Web builds (5.9 only, needs a source build and a C++ project).

The downsides:
- The authors themselves: made in a week, QA shorter than usual, "should be considered as unstable". They advise working on copies of projects.
- Known editor crashes: Python edit of a blueprint root, undo in Deformer Graph.
- In new projects the toolsets are off, All Toolsets has to be enabled by hand.
- Installed from "Other builds" in the launcher, for some people it did not show up.

**Recommendation: stay on 5.8.4.** Our bridge `F:\projects\unreal-mcp-bridge`, `MCP_Guideline.md` with a register of known bugs, build and packaging scripts are tuned on 5.8. 6 hours before the start and it is night now, no time to break in 5.9. A web build is not needed. Reconsider only if someone on the team has already worked in 5.9.

## 9. What we already have

Can be brought into the jam project (tools, fit rule 7(h), declare everything):
- MCP: bridge `F:\projects\unreal-mcp-bridge\`, take port 8002 (8000 Diskworld, 8001 CastleFight). `Diskworld\_Docs\30_Workflows\MCP_Guideline.md`, `How_to_Use_Native_MCP.md`, `Tools\Cutscene\mcp_direct.py`.
- Build: `build_and_test.sh`, `package.sh` (Win64 Development/Shipping + zip), `smoke_package.sh`, `check_code.py`. Build hand-out checklist in `Diskworld\_Docs\30_Workflows\Demo_Build.md`.
- `.claude`: `rules\cpp.md`, `rules\tools.md`, hook `unverified_code.py`, agent `lead.md`, command `docs-sync`, skill `create-toolset`.
- Generation: images through OpenAI (`gen_concepts.py` with a spend limit), sound and voice through ElevenLabs (`Diskworld\Tools\Voice\lib\`, `sfx_take.mjs`, `line_take.mjs`), Tripo import and retarget (`import_tripo.py`, `retarget_tripo_rig.py`), Blender MCP.
- Video: `Diskworld\Tools\Cutscene\mrq_render.py`, `Shorts\captions_ass.py`, `How_to_Record_Devlog.md`.
- Parallel agents: `How_to_Run_Parallel_Tickets.md`, `How_to_Share_Editor.md`.

Must not be brought: `Source\`, `Content\`, the Skelot plugin with its settings, models, sounds, UI, maps of both projects. Scripts from `Tools\Discovery\` are mostly tied to specific assets.

Proven skills: deterministic simulation in pure Rules under TDD, a crowd of ~400 units, network code, UMG, the image → Tripo → Blender → UE pipeline, ElevenLabs, cutscenes and camera in Sequencer, voice-over and dialogue, a Shipping build for testers.

## 10. Where the sources differ from the original list

Your list is right overall. What did not match or was missing:

1. **Development or Shipping.** On itch, next to desktop and mobile, it says "(development build)", while Web requires Shipping. It is unclear whether the note applies to all platforms or only to iOS. **Not verified word for word**, this item on the page has to be read by eye. The PDF does not specify the build type.
2. **Own assets made before the start.** The itch FAQ: "declare any prompts, scripts, or assets you made before the jam", as if your old assets are allowed if declared. PDF 7(h) is stricter: all art except third-party is made within the jam. On a conflict the PDF wins. We do not take our own art from CastleFight and Diskworld.
3. **Rule 7(h) on VCS history** covers materials made before the start (tools, prompts), not the whole game. The conclusion is the same: git from the first minute, and before the start a separate "tools only" commit.
4. **NVIDIA RTX Award** is on itch, but our PDF (ed. 26.10) does not have it at all, nor the list of 45 restricted countries the page refers to. A fresh English edition of the rules is needed.
5. **Prize limit**: at most one main and one special prize per entry. This was not in the list.
6. **Fun tie-break** (Fun factor doubled).
7. **Web build** needs not just 5.9 Preview but a source build and a C++ project. Limits 500 MB total, 200 MB per file.
8. **Slips of the host on the stream**, trust the page: "starts 2 a.m. Eastern" (actually 12:00 AM), "build from November 15th" (12–15), "6,500 in cash" ($65,000).
9. **The daily stream schedule changed**, the new time was not found in text sources.
10. Google and OpenAI were named as partners on the stream, there are no perks from them on the page.

## 11. Advice from the stream

- Scope small, then polish: "a finished fun game beats an ambitious broken one".
- A test build on the second day.
- Keep notes for the dev log as you go.
- Submit a couple of hours before the deadline: slow upload, wrong file.
- Give agents concrete, detailed requests. Ask for variants and tuning knobs ("give me the controls to fine-tune this"), ask them to build helper menus and tools.
- Get a prototype up fast with an agent, finish the last 20% by hand: "that last 20% usually requires a lot of hands-on".
