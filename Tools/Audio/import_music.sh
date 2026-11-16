#!/usr/bin/env bash
# Imports the music stems into /Game/Audio/Music with the engine's ImportAssets commandlet (no open editor needed).
#   import_music.sh [settings.json]     (default Tools/Audio/import_music.json; file paths in it are relative to the root)
# Kirill's stems replace the stubs: put his WAVs under the same names, list them in a settings file, run this again.
set -u
UE="F:/Program Files/Epic Games/UE_5.8"
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd -W)"
SETTINGS="${1:-$ROOT/Tools/Audio/import_music.json}"
LOG="$ROOT/Saved/BuildCycle/import_music.log"
# The commandlet resolves relative file names from the engine's folder: give it a copy with absolute ones.
RESOLVED="$ROOT/Saved/BuildCycle/import_music.resolved.json"
python -c "import json,sys,os; d=json.load(open(sys.argv[1])); [g.__setitem__('Filenames',[os.path.join(sys.argv[2],f).replace(os.sep,'/') for f in g['Filenames']]) for g in d['ImportGroups']]; json.dump(d,open(sys.argv[3],'w'),indent=1)" "$SETTINGS" "$ROOT" "$RESOLVED"
MSYS2_ARG_CONV_EXCL="*" "$UE/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" "$ROOT/JamGame.uproject" -run=ImportAssets \
	-importsettings="$RESOLVED" -nosourcecontrol -unattended -nosplash -abslog="$LOG"
# Exit 1 with "Invalid Destination Path ()" is expected: the commandlet also checks the -dest it was not given.
echo "[import] exit $?"
grep -E "Imported|Error|Warning: .*(Import|Sound)" "$LOG" | cut -c1-200 | head -20
