#!/usr/bin/env bash
# Records a clip of the game playing itself: the bot drives the hero on the top-down camera.
#   demo_clip.sh [seconds] [output.mp4] [camera]
# camera: ignored since T-018 (only the top-down camera is left), kept so older calls still work.
# Opens a game window for the length of the clip plus the start-up: warn the author first.
# Extra game switches go in JAM_DEMO_ARGS, e.g. JAM_DEMO_ARGS=-JamDemoHoldUltimate (the bot never presses E).
# The game saves every frame as a PNG at a fixed 30 fps (see Source/JamGame/Public/Game/JamDemo.h), ffmpeg joins them.
set -u
UE="F:/Program Files/Epic Games/UE_5.8"
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd -W)"
SECONDS_TO_RUN="${1:-26}"
OUT="${2:-$ROOT/Saved/Demo/demo_$(date +%m%d_%H%M).mp4}"
# A fresh folder per run: frames of an earlier run must never pad out a shorter one.
FRAMES="$ROOT/Saved/Demo/frames_$(date +%m%d_%H%M%S)"
LOG="$ROOT/Saved/Demo/demo.log"
CAMERA="${3:-}"
mkdir -p "$FRAMES" "$(dirname "$OUT")"

# Git Bash would turn /Game/... into a path under its own install folder.
MSYS2_ARG_CONV_EXCL="*" "$UE/Engine/Binaries/Win64/UnrealEditor.exe" "$ROOT/JamGame.uproject" /Game/Maps/Arena -game -windowed \
	-resx=1280 -resy=720 -nosplash -unattended -JamDemo -JamCapture="$FRAMES" -JamDemoSeconds="$SECONDS_TO_RUN" -abslog="$LOG" \
	${CAMERA:+-JamDemoCamera="$CAMERA"} ${JAM_DEMO_ARGS:-}
echo "[demo] game exit $?"
grep -E "LogJam[A-Za-z]*: (Warning|Error)|Fatal error|Ensure condition failed" "$LOG" | cut -c1-200 | sort | uniq -c | head -12

echo "[demo] frames saved: $(ls "$FRAMES" | wc -l)"
COUNT=$((SECONDS_TO_RUN * 30))
ffmpeg -y -loglevel error -framerate 30 -i "$FRAMES/f_%05d.png" -frames:v "$COUNT" -c:v libx264 -pix_fmt yuv420p -crf 23 "$OUT"
echo "[demo] ffmpeg exit $?"
ls -la "$OUT"
