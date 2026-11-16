#!/usr/bin/env bash
# Starts the packaged game for some seconds and reports what its log says.
#   smoke_package.sh [seconds] [map path, e.g. /Game/Maps/L_Main]
# Without a map the game opens its default map. A Shipping build writes no log unless it was built with logging:
# then only "still running" or "exited by itself" is reported.
set -u
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd -W)"
SECONDS_TO_RUN="${1:-30}"
MAP="${2:-}"
GAME="$ROOT/Saved/Packaged/Windows"
EXE="$GAME/JamGame/Binaries/Win64/JamGame.exe"
LOG="$GAME/JamGame/Saved/Logs/JamGame.log"
# A Shipping target is named JamGame-Win64-Shipping.exe.
[ -f "$EXE" ] || EXE="$(ls "$GAME"/JamGame/Binaries/Win64/JamGame*.exe 2>/dev/null | head -1)"
if [ -z "$EXE" ] || [ ! -f "$EXE" ]; then
	echo "[smoke] no packaged game under $GAME: run Tools/Build/package.sh first"
	exit 1
fi

if tasklist 2>/dev/null | grep -qi "JamGame"; then
	echo "[smoke] the game is already running, not starting a second one"
	exit 1
fi
rm -f "$LOG"
# Git Bash would turn /Game/... into a path under its own install folder.
MSYS2_ARG_CONV_EXCL="*" "$EXE" $MAP -windowed -resx=1280 -resy=720 -nosplash >/dev/null 2>&1 &
PID=$!
for _ in $(seq 1 "$SECONDS_TO_RUN"); do
	kill -0 "$PID" 2>/dev/null || break
	sleep 1
done
if kill -0 "$PID" 2>/dev/null; then
	echo "[smoke] still running after $SECONDS_TO_RUN s"
	# Only its own process: the author may be playing another copy of the build.
	kill "$PID" 2>/dev/null
else
	echo "[smoke] exited by itself"
fi
sleep 2
if [ -f "$LOG" ]; then
	wc -l "$LOG"
	grep -E "LoadMap|Browse:|Failed to load|Fatal error|Assertion failed|Unhandled Exception|LogJam[A-Za-z]*: (Warning|Error)|Error:" "$LOG" | cut -c1-200 | sort | uniq -c | sort -rn | head -24
else
	echo "[smoke] no log at $LOG"
fi
