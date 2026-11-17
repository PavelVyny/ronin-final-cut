# RONIN: FINAL CUT

A top-down boss fight made solo for the **Epic AI MegaJam 2026** (2026-11-12 05:00 UTC to 2026-11-16 01:59 UTC) in
Unreal Engine 5.8. A ronin fights the Publisher on a rooftop shrine over a rainy neon city. Three times in the fight
the Publisher takes one of two features you choose to sacrifice, turns it against you, and cuts the whole game down a
tier: full art, shape ronin, greybox, the bean. Drop his health to zero and everything snaps back for the final blow.

- Play it (Windows build): https://pavyny.itch.io/ronin-final-cut
- Dev log: [Directing the cut: four days, one boss and an AI agent](https://pavyny.itch.io/ronin-final-cut/devlog/1781307/directing-the-cut-four-days-one-boss-and-an-ai-agent)

This repository is the Glass Box copy of the project: the code, the tooling, the plans and every agent session, so
the judges can see how the game was made. The art, audio and levels are left out for size; the playable build is on
itch.

## How the project is organised

| Path | What it holds |
|---|---|
| `Source/JamGame/` | The game module. `Core` and `Sim` are the fight simulation (no engine actors, no presentation); `Game`, `View`, `UI` and `Audio` draw and play it. Decisions live in pure `*Rules` namespaces. |
| `Source/JamGame/Private/Tests/` | The `Jam.*` automation tests, written before the code they cover (323 at the end of the jam). |
| `Config/` | Engine and game settings; the balance numbers and the degradation tiers are in `DefaultGame.ini`. |
| `Tools/` | Build, test and packaging scripts (`Tools/Build`), the code checker (`Tools/Quality`), the editor scripts that build materials and import assets (`Tools/Editor`), the art and audio pipelines (`Tools/Art`, `Tools/Audio`), the session log exporter (`Tools/Devlog`) and the git hooks (`Tools/Git`). |
| `Tickets/` | One file per task (T-001 onwards): what was asked, what was done, how it was checked. |
| `.claude/` | The agent's C++ rules and hooks. |
| `DEVLOG.md` | The dev log written during the jam, a few lines per working block. |
| `PREJAM.md` | Everything that existed before the jam started. |
| `THIRD_PARTY.md` | Every tool, model, plugin and asset the project uses that was not made during the jam, with source and licence. |
| `CLAUDE.md`, `JAM_PLAN.md`, `JAM_BRIEF.md`, `HANDOFF.md`, `QUESTIONS.md`, `JAM_START_PROMPT.md` | The agent's instructions, the plan, the rules brief, hand-off notes between sessions. |

## Agent logs

Every Claude Code session of the project is in [`_AgentLogs/`](_AgentLogs/): `INDEX.md` lists them with their start
and end times (UTC) and whether they ran before or during the jam; `md/` holds readable transcripts and `raw/` the
original JSONL, subagents included. The git pre-commit hook (`Tools/Git/hooks/pre-commit`) exported them on every
commit. One value was redacted as `[REDACTED]`: the Android File Server's `SecurityToken` from
`Config/DefaultEngine.ini`, where the logs quote it.

The logs are kept exactly as recorded. The sessions ran in English, but the author's native language is Russian, so a
few of the author's messages in them are in Russian. They also mention CastleFight and Diskworld, the author's other
Unreal projects, where the pre-jam tooling came from (see `PREJAM.md`). `JAM_BRIEF.md` and `JAM_PLAN.md` were written
in Russian and translated to English after the jam; the originals are in the first commit.

## Before the jam

Only tooling existed before the start (see `PREJAM.md`). The pre-jam commits in the working repository
(`git log --grep "pre-jam:"`):

| Commit | Date | Subject |
|---|---|---|
| `8a5a676` | 2026-11-12 03:49:07 +0200 (01:49 UTC) | pre-jam: tooling and plan |

This repository is a single-commit export of the working one; the full history with one commit per change stays in
the author's working repository.

## What is left out

- `Content/`, `Art/`, `Audio/` and `Clips/`: the assets, for size. The playable build is on itch.
- `Binaries/`, `Intermediate/`, `Saved/`, `DerivedDataCache/`: build output.
- `Plugins/`: the only project plugins are NVIDIA's DLSS (`DLSS` and `StreamlineNGXCommon`, enabled in
  `JamGame.uproject`). They are NVIDIA's, so they are not redistributed here: get the NVIDIA DLSS plugin for Unreal
  Engine 5.8 from NVIDIA's developer site (https://developer.nvidia.com/rtx/dlss) and put it in `Plugins/`, or remove
  the two entries from `JamGame.uproject`.
- Any file over 50 MB.

Without the content there is nothing to play: use the itch build for that. The build and test cycle the agent used is
`bash Tools/Build/build_and_test.sh`.
