#!/usr/bin/env bash
# Records the trailer's raw footage from the packaged Shipping game: the bot plays a whole fight in 1920x1080, a cut
# every SKIP seconds, so the steal cards, the cracks and the fall from full art to greybox all come in one run.
#   trailer_record.sh <packaged Windows folder> [seconds] [output.mp4]
# Opens a game window for the length of the run: warn the author first. Frames are PNGs at a fixed 30 fps
# (Source/JamGame/Public/Game/JamDemo.h), about 3 MB each at 1080p; ffmpeg joins them, the frames stay for a re-cut.
set -u
PACKAGED="${1:?packaged Windows folder}"
SECONDS_TO_RUN="${2:-80}"
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd -W)"
OUT="${3:-$ROOT/Saved/Demo/trailer_raw.mp4}"
FRAMES="$ROOT/Saved/Demo/trailer_frames_$(date +%m%d_%H%M%S)"
LOG="$ROOT/Saved/Demo/trailer.log"
# A cut every 12 s; the Publisher gets the Ultimate and the Parry when they are on the card, so he uses them on screen.
ARGS="-JamDemo -JamDemoNoIntro -JamSeed=7 -JamDemoSkipEvery=12 -JamDemoSteal=ULTIMATE,PARRY"
mkdir -p "$FRAMES" "$(dirname "$OUT")"
EXE="$(ls "$PACKAGED"/*.exe | head -1)"
echo "[trailer] $EXE"
"$EXE" -windowed -resx=1920 -resy=1080 -nosplash -JamCapture="$FRAMES" -JamDemoSeconds="$SECONDS_TO_RUN" -abslog="$LOG" $ARGS
echo "[trailer] game exit $?"
echo "[trailer] frames saved: $(ls "$FRAMES" | wc -l) in $FRAMES"
ffmpeg -y -loglevel error -framerate 30 -i "$FRAMES/f_%05d.png" -c:v libx264 -pix_fmt yuv420p -crf 18 "$OUT"
echo "[trailer] ffmpeg exit $?"
ls -la "$OUT"
