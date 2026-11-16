# T-017 Checkpoint K1: Shipping build on itch, a clip per camera

**Block:** K1 · **Status:** doing · **Deadline:** Thu 23:00 (PC clock)

## Goal

The author downloads the build from the itch draft on Windows, plays the fight from start to a title and picks one
camera by Friday 07:00.

## Steps

1. `bash Tools/Build/build_and_test.sh` green.
2. `bash Tools/Build/package.sh --config Shipping --name <TeamName>_NeonRonin`.
3. Warn the author, then `bash Tools/Build/smoke_package.sh` (it opens a game window).
4. `butler push` of the packaged folder to the draft page, channel `windows` (needs the page address from the author;
   until then the zip is sent as a file).
5. Three clips of 30 to 60 seconds, one per camera, recorded with the bot playing. Sent to the author.
6. `DEVLOG.md`: what the build contains, what is known to be broken, the question "which camera?".

## Test seam

None of its own: this ticket ships what the other tickets tested. The smoke run is the check.

## Done

The build is on the draft page (or the zip is with the author), clips sent, the commit is tagged `k1`.

## As built (Thu 16:05)

- Package: Shipping, built from commit `136fe26` (107 `Jam.` tests green), tag `k1` on that commit. Zip
  `Saved/Packaged/NeonRonin_K1.zip`, 303 MB; the name is a placeholder until the team name is known.
- Smoke: the packaged exe ran the bot fight for 8 seconds and closed by itself; the frames show the arena, both
  fighters, a volley, the slam warning and the HUD.
- Clips, 50 seconds each, sent to the author: `Clips/k1_cam0_topdown.mp4`, `Clips/k1_cam1_shoulder.mp4`,
  `Clips/k1_cam2_firstperson.mp4`.
- **Open: the build is not with the author yet.** The itch push waits for the page address (Q4), and the zip is
  over the 30 MB limit of the file channel. The ticket stays `doing` until one of the two works.
