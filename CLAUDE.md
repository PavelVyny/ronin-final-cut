# MegaJam 2026: project rules

Everything is in **English**: the conversation with the author, code, comments, commits, the dev log and every text
in the game (jam rule: the submission must be playable and judged in English; the session logs are part of the dev
log). The author may write in Russian; reply in English anyway.

## Hard rules

1. **Jam window: 2026-11-12 05:00 UTC to 2026-11-16 01:59 UTC.** Everything in the game (gameplay code, levels,
   art, sound) is made inside it. Before the start only tooling exists: see `PREJAM.md`. Rules and deadlines are in
   `JAM_BRIEF.md`.
2. **Engine: UE 5.8**, module `JamGame`, tests `Jam.<Area>.<Case>`, log category `LogJamGame`.
3. **Build and tests: `bash Tools/Build/build_and_test.sh`.** The script closes only this project's editor, the full
   build needs no permission. New `check_code.py` warnings are fixed or justified in one line.
4. **The MCP server of this project is on port 8002** (Diskworld 8000, CastleFight 8001).
5. **Version control: git + LFS.** Commit after Save All in the editor, otherwise an old `.uasset` gets in. Never
   amend: each commit is evidence of when the work was done.
6. **Agent logs are part of the submission evidence.** `Tools/Git/hooks/pre-commit` exports every Claude Code
   session of this project to `_AgentLogs/` on each commit. Enable once per clone:
   `git config core.hooksPath Tools/Git/hooks`. A session run from another folder is listed in
   `Tools/Devlog/sessions.json`.
7. **Third-party content is declared.** Every asset, plugin, model or sound that the team did not make during the
   jam goes to `THIRD_PARTY.md` with its source and licence at the moment it is added.
8. **Deleting files** only through a script with a dry run by default and the author's explicit yes.

## Working cycle

- Production code is written test-first: a `Jam.` test before the code, decisions in pure `*Rules`
  (`.claude/rules/cpp.md`).
- Packaging: `bash Tools/Build/package.sh --config Shipping --name TeamName_GameName`, then
  `bash Tools/Build/smoke_package.sh`. Warn the author before a smoke run: it opens a game window.
- The dev log is `DEVLOG.md`: a few lines per working block (what was done, which AI tool, what went wrong).
