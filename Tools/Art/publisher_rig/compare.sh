#!/usr/bin/env bash
# KIND: offline tool. Renders the same frames of one clip from the Meshy file and from the reworked file, one sheet each.
#   bash Tools/Art/publisher_rig/compare.sh <out_dir> <clip> <frames,comma> [views=front,q3] [size=480]
set -euo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../../.." && pwd)"
BLENDER="${BLENDER:-/f/Program Files/Blender Foundation/Blender 5.2/blender.exe}"
OUT="$1"; CLIP="$2"; AT="$3"; VIEWS="${4:-front,q3}"; SIZE="${5:-480}"
COLS="$(echo "$AT" | tr ',' '\n' | wc -l)"
mkdir -p "$OUT"
"$BLENDER" -b --factory-startup --python "$HERE/render_poses.py" -- "$ROOT/Art/Models/boss_clips/result.animation_fbx_url.fbx" "$OUT/before" --clips "$CLIP" --at "$AT" --views "$VIEWS" --size "$SIZE" > "$OUT/before.log" 2>&1
"$BLENDER" -b --factory-startup --python "$HERE/render_poses.py" -- "$ROOT/Art/Models/boss_fixed/publisher_clips.blend" "$OUT/after" --clips "$CLIP" --at "$AT" --views "$VIEWS" --size "$SIZE" > "$OUT/after.log" 2>&1
python "$HERE/sheet.py" "$OUT/before.png" --cols "$COLS" --cell "$SIZE" "$OUT/before/"*.png
python "$HERE/sheet.py" "$OUT/after.png" --cols "$COLS" --cell "$SIZE" "$OUT/after/"*.png
