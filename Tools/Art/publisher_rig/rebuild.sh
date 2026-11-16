#!/usr/bin/env bash
# KIND: offline tool. Rebuilds the Publisher rig and its clips from the Meshy files, then reports the skin tears of
# one pose.
#   bash Tools/Art/publisher_rig/rebuild.sh <work_dir> [clip=Idle] [frame=1]
set -euo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../../.." && pwd)"
BLENDER="${BLENDER:-/f/Program Files/Blender Foundation/Blender 5.2/blender.exe}"
OUT="$1"; CLIP="${2:-Idle}"; FRAME="${3:-1}"
FIXED="$ROOT/Art/Models/boss_fixed"
mkdir -p "$OUT"
"$BLENDER" -b --factory-startup --python "$HERE/build_rig.py" > "$OUT/build.log" 2>&1
"$BLENDER" -b --factory-startup --python "$HERE/plain_shirt.py" -- "$FIXED/publisher_rig.blend" --write "$OUT/shirt.txt" > "$OUT/shirt.log" 2>&1
"$BLENDER" -b --factory-startup --python "$HERE/rework_clips.py" -- "$FIXED/publisher_rig.blend" "$FIXED/publisher_clips.blend" "$FIXED/clips.json" "$OUT/rework.txt" > "$OUT/rework.log" 2>&1
"$BLENDER" -b --factory-startup --python "$HERE/find_tears.py" -- "$FIXED/publisher_clips.blend" "$OUT/tears.txt" "$CLIP" "$FRAME" > "$OUT/tears.log" 2>&1
grep "\[build\]" "$OUT/build.log" || true
head -12 "$OUT/tears.txt"
