#!/usr/bin/env bash
# KIND: offline tool. Renders a few frames of one clip large and crops the chest and head into one sheet.
#   bash Tools/Art/publisher_rig/closeup.sh <out_dir> <clip> <frames,comma> [file.blend] [views=front,q3]
set -euo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../../.." && pwd)"
BLENDER="${BLENDER:-/f/Program Files/Blender Foundation/Blender 5.2/blender.exe}"
OUT="$1"; CLIP="$2"; AT="$3"; FILE="${4:-$ROOT/Art/Models/boss_fixed/publisher_clips.blend}"; VIEWS="${5:-front,q3}"
"$BLENDER" -b --factory-startup --python "$HERE/render_poses.py" -- "$FILE" "$OUT/full" --clips "$CLIP" --at "$AT" --views "$VIEWS" --size 1400 > "$OUT.log" 2>&1
mkdir -p "$OUT/crop"
python -c "import sys,glob,os;from PIL import Image;[Image.open(f).crop((350,120,1050,720)).save(os.path.join(sys.argv[2],os.path.basename(f))) for f in glob.glob(sys.argv[1]+'/*.png')]" "$OUT/full" "$OUT/crop"
python "$HERE/sheet.py" "$OUT/sheet.png" --cols 4 --cell 480 "$OUT/crop/"*.png
