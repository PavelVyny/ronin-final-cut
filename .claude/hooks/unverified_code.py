"""
KIND: Claude Code Stop hook (read-only except its own reminder state).

Reminds the agent once per set of changes that Source/ or Tools/ changed after the last green cycle of
Tools/Build/build_and_test.sh (green_source.stamp: Jam. tests passed; green_tools.stamp: Tools pytest passed).
The reminder goes back to the agent as additionalContext, which gives it one more turn: run the cycle, or say
in one line why not (the work is not finished). It never blocks twice for the same changes.
"""
import json
import os
import sys

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", ".."))
CYCLE = os.path.join(ROOT, "Saved", "BuildCycle")
STATE = os.path.join(CYCLE, "stop_hook_reminded.json")
SKIP_DIRS = {"__pycache__", ".pytest_cache", "node_modules", "out"}
AREAS = {
    "Source": ((".h", ".cpp", ".cs"), "green_source.stamp", "bash Tools/Build/build_and_test.sh"),
    "Tools": ((".py", ".mjs", ".js", ".sh"), "green_tools.stamp", "bash Tools/Build/build_and_test.sh --no-build --no-test"),
}


def newest(area, extensions):
    best = 0.0
    for d, dirs, files in os.walk(os.path.join(ROOT, area)):
        dirs[:] = [x for x in dirs if x not in SKIP_DIRS]
        for f in files:
            if f.endswith(extensions):
                best = max(best, os.path.getmtime(os.path.join(d, f)))
    return best


def mtime(path):
    return os.path.getmtime(path) if os.path.exists(path) else 0.0


def main():
    try:
        event = json.load(sys.stdin)
    except ValueError:
        event = {}
    if event.get("stop_hook_active"):
        return 0
    try:
        with open(STATE, encoding="utf-8") as h:
            reminded = json.load(h)
    except (OSError, ValueError):
        reminded = {}

    lines = []
    for area, (extensions, stamp, command) in AREAS.items():
        changed = newest(area, extensions)
        if changed > max(mtime(os.path.join(CYCLE, stamp)), reminded.get(area, 0.0)):
            lines.append("%s/ changed after the last green cycle: run `%s`." % (area, command))
            reminded[area] = changed
    if not lines:
        return 0

    os.makedirs(CYCLE, exist_ok=True)
    with open(STATE, "w", encoding="utf-8") as h:
        json.dump(reminded, h)
    text = " ".join(lines) + (
        " Then handle every new check_code.py warning (fix it or justify it in one line). Closing the editor and"
        " a full build need no permission. If the work is not finished yet, say so in one line and stop."
        " Reminder from .claude/hooks/unverified_code.py, once per set of changes.")
    print(json.dumps({"hookSpecificOutput": {"hookEventName": "Stop", "additionalContext": text}}))
    return 0


if __name__ == "__main__":
    sys.exit(main())
