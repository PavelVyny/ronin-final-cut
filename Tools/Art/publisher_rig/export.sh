#!/usr/bin/env bash
# KIND: offline tool. Exports the Publisher FBX files into Art/Models/boss_fixed and reads them back as a check.
#   bash Tools/Art/publisher_rig/export.sh <work_dir>
set -euo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../../.." && pwd)"
BLENDER="${BLENDER:-/f/Program Files/Blender Foundation/Blender 5.2/blender.exe}"
OUT="$1"
FIXED="$ROOT/Art/Models/boss_fixed"
mkdir -p "$OUT"
"$BLENDER" -b --factory-startup --python "$HERE/export_fbx.py" -- "$FIXED/publisher_clips.blend" "$FIXED" "$OUT/probe.json" > "$OUT/export.log" 2>&1
grep "\[fbx\]" "$OUT/export.log"
"$BLENDER" -b --factory-startup --python "$HERE/verify_fbx.py" -- "$FIXED" "$OUT/probe.json" "$FIXED/export_check.txt" > "$OUT/verify.log" 2>&1
cat "$FIXED/export_check.txt"
