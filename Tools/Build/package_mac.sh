#!/usr/bin/env bash
# Packages the jam project for Mac on a Mac with Xcode and UE 5.8 (a Windows PC cannot do this).
#   bash Tools/Build/package_mac.sh [Development|Shipping]
# The playable app is the staged one (Saved/StagedBuilds/Mac); Binaries/Mac/JamGame.app has no content and does not start.
set -u
UE="${UE_ROOT:-/Users/Shared/Epic Games/UE_5.8}"
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
STAGED="$ROOT/Saved/StagedBuilds/Mac"
OUT="$ROOT/Saved/Packaged"
LOG="$ROOT/Saved/package_mac.log"
CONFIG="${1:-Development}"
mkdir -p "$OUT"

if [ ! -x "$UE/Engine/Build/BatchFiles/RunUAT.sh" ]; then
	echo "[package] no UE 5.8 at $UE, run: UE_ROOT=/path/to/UE_5.8 bash Tools/Build/package_mac.sh"
	exit 1
fi
echo "[package] Mac $CONFIG, log: $LOG"
"$UE/Engine/Build/BatchFiles/RunUAT.sh" BuildCookRun -project="$ROOT/JamGame.uproject" -noP4 -platform=Mac -clientconfig="$CONFIG" \
	-build -cook -stage -pak -unattended -utf8output >"$LOG" 2>&1
CODE=$?
grep -E "BUILD SUCCESSFUL|BUILD FAILED|AutomationTool exiting|Error:" "$LOG" | tail -12
if [ "$CODE" != 0 ]; then
	echo "[package] FAILED, exit $CODE. Send $LOG"
	exit "$CODE"
fi
APP="$(find "$STAGED" -maxdepth 2 -name "*.app" | head -1)"
if [ -z "$APP" ]; then
	echo "[package] FAILED, no .app under $STAGED"
	exit 1
fi
ZIP="$OUT/JamGame-Mac-$CONFIG.zip"
rm -f "$ZIP"
ditto -c -k --keepParent "$APP" "$ZIP" || exit 1
echo "[package] ok: $APP"
echo "[package] zip: $ZIP"
