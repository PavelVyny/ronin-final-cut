#!/usr/bin/env bash
# KIND: offline tool. Renders key frames of the rebuilt rig and joins them into one sheet per clip.
#   bash Tools/Art/publisher_rig/preview.sh <out_dir> "<clips,comma>" [frames=6] [views=q3,q3b] [file.blend] [cell=320]
set -euo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../../.." && pwd)"
BLENDER="${BLENDER:-/f/Program Files/Blender Foundation/Blender 5.2/blender.exe}"
OUT="$1"; CLIPS="$2"; FRAMES="${3:-6}"; VIEWS="${4:-q3,q3b}"; FILE="${5:-$ROOT/Art/Models/boss_fixed/publisher_rig.blend}"; CELL="${6:-320}"
"$BLENDER" -b --factory-startup --python "$HERE/render_poses.py" -- "$FILE" "$OUT/key" --clips "$CLIPS" --frames "$FRAMES" --views "$VIEWS" --size 512 2>&1 | grep -i "error\|traceback\|done" || true
IFS=',' read -ra LIST <<< "$CLIPS"
IFS=',' read -ra VLIST <<< "$VIEWS"
for c in "${LIST[@]}"; do
  files=()
  for v in "${VLIST[@]}"; do
    for f in "$OUT/key/${c}_${v}_"*.png; do files+=("$f"); done
  done
  python "$HERE/sheet.py" "$OUT/sheet_$c.png" --cols "$FRAMES" --cell "$CELL" --title "$c" "${files[@]}"
done
