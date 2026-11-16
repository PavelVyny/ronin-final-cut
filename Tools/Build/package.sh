#!/usr/bin/env bash
# Packages the jam project for Win64 into Saved/Packaged/Windows and zips it next to the folder.
#   package.sh [--config Development|Shipping] [--name ZipBaseName] [--no-zip]
# Builds from the working tree. The editor can stay open: only the game target is compiled.
# The submission zip must be named TeamName_GameName: pass it with --name.
set -u
UE="F:/Program Files/Epic Games/UE_5.8"
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd -W)"
OUT="$ROOT/Saved/Packaged"
LOG="$ROOT/Saved/BuildCycle/package_$(date +%H%M%S).log"
CONFIG=Development
NAME=""
DO_ZIP=1
while [ $# -gt 0 ]; do
	case "$1" in
		--config) shift; CONFIG="$1" ;;
		--name) shift; NAME="$1" ;;
		--no-zip) DO_ZIP=0 ;;
	esac
	shift
done
[ -z "$NAME" ] && NAME="JamGame_$CONFIG"

mkdir -p "$OUT" "$ROOT/Saved/BuildCycle"
echo "[package] $CONFIG, log: $LOG"
"$UE/Engine/Build/BatchFiles/RunUAT.bat" BuildCookRun -project="$ROOT/JamGame.uproject" -noP4 -platform=Win64 -clientconfig="$CONFIG" \
	-build -cook -stage -pak -prereqs -archive -archivedirectory="$OUT" -nocompileeditor -ubtargs=-WaitMutex -unattended -utf8output >"$LOG" 2>&1
CODE=$?
grep -E "BUILD SUCCESSFUL|BUILD FAILED|AutomationTool exiting|Error:" "$LOG" | tail -12
if [ "$CODE" != 0 ]; then
	echo "[package] FAILED, exit $CODE"
	exit "$CODE"
fi
# Tells builds apart when a tester reports a problem.
echo "$(git -C "$ROOT" rev-parse --short HEAD 2>/dev/null)-$(date +%m%d%H%M)" > "$OUT/Windows/JamGame/BuildVersion.txt"
du -sh "$OUT/Windows"
if [ "$DO_ZIP" = 1 ]; then
	# Without Saved: settings and logs of runs on this PC do not belong in the build.
	/c/Windows/System32/tar.exe -a -c -f "$OUT/$NAME.zip" --exclude "JamGame/Saved" -C "$OUT/Windows" .
	ls -la "$OUT/$NAME.zip"
fi
echo "[package] ok: $OUT/Windows"
