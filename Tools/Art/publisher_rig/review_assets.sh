#!/usr/bin/env bash
# KIND: offline tool. Renders the review material of the Publisher rig: per clip a before/after animation and an
# after key-frame sheet.
#   bash Tools/Art/publisher_rig/review_assets.sh <work_dir>
set -euo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../../.." && pwd)"
BLENDER="${BLENDER:-/f/Program Files/Blender Foundation/Blender 5.2/blender.exe}"
OUT="$1"
CLIPS="Idle,Slow_Orc_Walk,Heavy_Hammer_Swing,Kung_Fu_Punch,Charged_Ground_Slam,Hit_Reaction,Knock_Down"
BEFORE="$ROOT/Art/Models/boss_clips/result.animation_fbx_url.fbx"
AFTER="$ROOT/Art/Models/boss_fixed/publisher_clips.blend"
mkdir -p "$OUT/anim"
"$BLENDER" -b --factory-startup --python "$HERE/render_poses.py" -- "$BEFORE" "$OUT/before_seq" --clips "$CLIPS" --step 2 --views q3 --size 320 > "$OUT/before_seq.log" 2>&1
"$BLENDER" -b --factory-startup --python "$HERE/render_poses.py" -- "$AFTER" "$OUT/after_seq" --clips "$CLIPS" --step 2 --views q3 --size 320 > "$OUT/after_seq.log" 2>&1
IFS=',' read -ra LIST <<< "$CLIPS"
for c in "${LIST[@]}"; do
  python "$HERE/sheet.py" "$OUT/anim/$c.webp" --fps 15 --pair "$OUT/before_seq" "$OUT/after_seq" "${c}_q3_"
done
bash "$HERE/preview.sh" "$OUT/after" "$CLIPS" 6 q3,q3b "$AFTER" 320
echo "review assets done"
