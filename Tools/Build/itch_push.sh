#!/usr/bin/env bash
# Pushes a packaged folder to the itch page pavyny/ronin-final-cut, channel windows. Key from .env, never printed.
#   itch_push.sh <folder> <version>     e.g. itch_push.sh Saved/Packaged/Windows block-d
# Needs butler in ~/bin (v15.32.0 installed Sat 14 Nov). Pipe the output through a filter before showing it.
set -euo pipefail
cd /f/Unreal_Engine/UnrealProjects/MegaJam2026
BUTLER_API_KEY="$(grep '^BUTLER_API_KEY=' .env | head -1 | cut -d= -f2- | tr -d '\r"')"
export BUTLER_API_KEY
~/bin/butler.exe push "$1" pavyny/ronin-final-cut:windows --userversion "$2"
~/bin/butler.exe status pavyny/ronin-final-cut
