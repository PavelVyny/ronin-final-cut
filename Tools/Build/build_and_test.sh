#!/usr/bin/env bash
# Autonomous build + automation test cycle for the jam project (ported from the CastleFight and Diskworld tooling).
#   build_and_test.sh [--no-build] [--no-test] [--no-check] [--open] [--filter Jam.]
# Closes this checkout's JamGame editor only (graceful first, forced after 30 s; editors of other worktrees and
# projects stay open),
# runs the full editor build, runs the project's automation tests headless and prints a summary, then the
# Tools pytest suite and the code checks (Tools/Quality/check_code.py, warnings only).
# --open relaunches the editor afterwards; ignored in a linked git worktree.
set -u
UE="F:/Program Files/Epic Games/UE_5.8"
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd -W)"
PROJ="$ROOT/JamGame.uproject"
LOGDIR="$ROOT/Saved/Logs"
OUT="$ROOT/Saved/BuildCycle"
mkdir -p "$OUT"

DO_BUILD=1; DO_TEST=1; DO_CHECK=1; DO_OPEN=0; FILTER="Jam."
while [ $# -gt 0 ]; do
	case "$1" in
		--no-build) DO_BUILD=0 ;;
		--no-test) DO_TEST=0 ;;
		--no-check) DO_CHECK=0 ;;
		--open) DO_OPEN=1 ;;
		--filter) shift; FILTER="$1" ;;
	esac
	shift
done

# A linked git worktree never opens an editor: there is one MCP port and the main
# checkout owns it.
# Both paths absolute: plain --git-dir is relative in the main checkout and absolute in a worktree.
if [ "$(git -C "$ROOT" rev-parse --path-format=absolute --git-dir 2>/dev/null)" != "$(git -C "$ROOT" rev-parse --path-format=absolute --git-common-dir 2>/dev/null)" ]; then
	IS_WORKTREE=1
else
	IS_WORKTREE=0
fi

# PIDs of UnrealEditor processes opened on this checkout's JamGame.uproject. Matched by full path, so editors of
# other worktrees and of other projects stay open.
editor_pids() {
	powershell -NoProfile -Command "\$p = '$PROJ'.ToLower(); Get-CimInstance Win32_Process -Filter \"Name='UnrealEditor.exe'\" | Where-Object { \$_.CommandLine -and \$_.CommandLine.Replace('\\', '/').ToLower().Contains(\$p) } | ForEach-Object { \$_.ProcessId }" 2>/dev/null | tr -d '\r'
}
editor_running() { [ -n "$(editor_pids)" ]; }
kill_editor() { for pid in $(editor_pids); do taskkill $1 //PID "$pid" >/dev/null 2>&1; done; }

if [ "$DO_BUILD" = 0 ] && [ "$DO_TEST" = 0 ]; then
	: # checks only: the editor stays open
elif editor_running; then
	echo "[cycle] closing the JamGame editor"
	kill_editor ""
	for i in $(seq 1 30); do editor_running || break; sleep 1; done
	if editor_running; then
		echo "[cycle] editor did not close in 30 s, forcing"
		kill_editor "//F"
		# A forced kill still takes a few seconds to release the process.
		for i in $(seq 1 15); do editor_running || break; sleep 1; done
	fi
	editor_running && { echo "[cycle] ERROR: editor still running"; exit 2; }
fi

if [ "$DO_BUILD" = 1 ]; then
	STAMP=$(date +%H%M%S)
	echo "[cycle] building (log: $OUT/build_$STAMP.log)"
	# -NoHotReloadFromIDE: UBT's Live Coding guard keys on the shared engine UnrealEditor.exe, so another project's open
	# editor would block this build (HotReload.cs, CheckForLiveCodingSessionActive). Safe: our editor is closed above.
	"$UE/Engine/Build/BatchFiles/Build.bat" JamGameEditor Win64 Development -Project="$PROJ" -WaitMutex -NoHotReloadFromIDE > "$OUT/build_$STAMP.log" 2>&1
	RC=$?
	grep -E "error C|error LNK|fatal error|: error|Error:" "$OUT/build_$STAMP.log" | head -40
	grep -cE "warning C" "$OUT/build_$STAMP.log" | sed 's/^/[cycle] compiler warnings: /'
	tail -3 "$OUT/build_$STAMP.log"
	if [ $RC -ne 0 ]; then echo "[cycle] BUILD FAILED rc=$RC"; exit 3; fi
	echo "[cycle] build ok"
fi

if [ "$DO_TEST" = 1 ]; then
	echo "[cycle] running automation tests: $FILTER"
	rm -f "$LOGDIR/AutoTests.log"
	# The test editor is not a commandlet, so it would auto-start the MCP server from the project ini and race the open
	# editor of the main checkout for port 8002 (ModelContextProtocolSettings.cpp, ShouldAutoStartServer).
	MCP_OFF="-ini:EditorPerProjectUserSettings:[/Script/ModelContextProtocolEngine.ModelContextProtocolSettings]:bAutoStartServer=False"
	"$UE/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" "$PROJ" "$MCP_OFF" -ExecCmds="Automation RunTests $FILTER" -unattended -nopause -nosplash -nullrhi -testexit="Automation Test Queue Empty" -log=AutoTests.log > "$OUT/tests_stdout.log" 2>&1
	LOG="$LOGDIR/AutoTests.log"
	if [ ! -f "$LOG" ]; then echo "[cycle] ERROR: no AutoTests.log"; exit 4; fi
	PASS=$(grep -c "Result={Success}" "$LOG")
	FAIL=$(grep -cE "Result=\{(Fail|Failed|Error)" "$LOG")
	echo "[cycle] tests: success=$PASS fail=$FAIL"
	grep -E "Result=\{(Fail|Failed|Error)" "$LOG" | head -20
	grep -E "Expected|Error: " "$LOG" | grep -v "Result=" | head -20
	cp "$LOG" "$OUT/AutoTests_last.log"
	[ "$FAIL" -gt 0 ] && { echo "[cycle] TESTS FAILED"; RCT=5; } || { RCT=0; touch "$OUT/green_source.stamp"; }
fi

if [ "$DO_CHECK" = 1 ]; then
	echo "[cycle] Tools pytest"
	# The stamps feed the Stop hook .claude/hooks/unverified_code.py.
	if python -m pytest "$ROOT/Tools/tests" -q -p no:cacheprovider > "$OUT/pytest.log" 2>&1; then
		touch "$OUT/green_tools.stamp"
	else
		echo "[cycle] PYTEST FAILED (see $OUT/pytest.log)"; RCT=6
	fi
	tail -1 "$OUT/pytest.log"
	TESTS_LOG=""
	[ "$DO_TEST" = 1 ] && TESTS_LOG="--tests-log $OUT/AutoTests_last.log"
	python "$ROOT/Tools/Quality/check_code.py" $TESTS_LOG
fi

if [ "$DO_OPEN" = 1 ] && [ "$IS_WORKTREE" = 1 ]; then
	echo "[cycle] --open ignored in a linked worktree"
elif [ "$DO_OPEN" = 1 ]; then
	echo "[cycle] opening the editor"
	# Detach the editor's handles from this script's output, or a caller piping it waits until the editor exits.
	cmd //c start "" "$UE/Engine/Binaries/Win64/UnrealEditor.exe" "$PROJ" </dev/null >/dev/null 2>&1
fi
exit ${RCT:-0}
