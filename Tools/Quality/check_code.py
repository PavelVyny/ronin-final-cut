"""
KIND: offline tool (read-only checks of Source/ against .claude/rules/cpp.md). Ported from the CastleFight and Diskworld tooling.

Every finding is a warning: the script always exits 0. A finding is new when a violation appears or grows
against Tools/Quality/baseline.json; the agent fixes it or justifies it in the session report.

    python Tools/Quality/check_code.py [--tests-log Saved/Logs/AutoTests.log] [--update-baseline] [--no-format]
    python Tools/Quality/check_code.py --report | --snapshot <milestone>   (milestone trend)

Needs the pinned tools: pip install -r Tools/Quality/requirements.txt
"""
import argparse
import collections
import json
import os
import re
import subprocess
import sys

import lizard

HERE = os.path.dirname(os.path.abspath(__file__))
SOURCE = os.path.normpath(os.path.join(HERE, "..", "..", "Source"))
MODULE = os.path.join(SOURCE, "JamGame")
BASELINE = os.path.join(HERE, "baseline.json")
SNAPSHOTS = os.path.join(HERE, "snapshots")

# Include directions that break the layering, as (from system, to system): the simulation never includes the
# presentation (cpp.md, systems and dependencies). Extend both lists when a new system folder appears.
SIMULATION = ("Core", "Sim")
PRESENTATION = ("View", "UI", "Audio")
SIMULATION_FORBIDDEN = frozenset((src, dst) for src in SIMULATION for dst in PRESENTATION)
FORBIDDEN = SIMULATION_FORBIDDEN

# Soft thresholds (cpp.md, decomposition): exceeding one is a signal, not a ban.
MAX_FUNCTION_LINES = 60
MAX_FILE_LINES = 600
MAX_NESTING = 3

INCLUDE = re.compile(r'#include "([^"]+)"')
# Systems that must reach actors through interfaces, tags or events, never by casting to a concrete class.
SHARED = set()
CONCRETE_ACTORS = []
LOGTEMP = re.compile(r"\bLogTemp\b")
PASSED_JAM_TEST = re.compile(r"Result=\{Success\}[^\n]*Path=\{Jam\.")
FORMAT_WARNING = re.compile(r"^(.+?\.(?:cpp|h)):\d+:\d+: warning:")


def concrete_cast(class_names):
    """Regex matching Cast<> to one of class_names, or None when there is nothing to check."""
    if not class_names:
        return None
    return re.compile(r"Cast<\s*(%s)\s*>" % "|".join(map(re.escape, class_names)))


CONCRETE_CAST = concrete_cast(CONCRETE_ACTORS)


def system_of(rel):
    """System of a module-relative path (Public/ and Private/ stripped): its top folder, None for root files."""
    parts = rel.replace("\\", "/").split("/")
    return parts[0] if len(parts) > 1 else None


def _walk(module_root):
    """(relative path, absolute path) of every file of the module; Public/ and Private/ prefixes are dropped."""
    for d, _, files in os.walk(module_root):
        for f in files:
            path = os.path.join(d, f)
            rel = os.path.relpath(path, module_root).replace("\\", "/")
            for side in ("Public/", "Private/"):
                if rel.startswith(side):
                    rel = rel[len(side):]
                    break
            yield rel, path


def _read(path):
    with open(path, encoding="utf-8", errors="ignore") as h:
        return h.read()


def dependency_violations(module_root):
    """{violation key: count} for the layering rules of the module at module_root."""
    files = list(_walk(module_root))
    owner = {rel: system_of(rel) for rel, _ in files}
    edges = collections.Counter()
    found = {}
    for rel, path in files:
        src = owner[rel]
        if src is None or src == "Tests":
            continue
        text = _read(path)
        if src in SHARED and CONCRETE_CAST is not None:
            casts = len(CONCRETE_CAST.findall(text))
            if casts:
                found["cast " + rel] = casts
        for target in INCLUDE.findall(text):
            dst = owner.get(target)
            if dst and dst != src and dst != "Tests":
                edges[(src, dst)] += 1
    for (src, dst), n in edges.items():
        if (src, dst) in FORBIDDEN:
            found["include %s -> %s" % (src, dst)] = n
        if (dst, src) in edges and src < dst:
            found["cycle %s <-> %s" % (src, dst)] = n + edges[(dst, src)]
    return found


def size_violations(module_root):
    """{violation key: measured value} for the soft size thresholds of every .cpp and .h of the module."""
    found = {}
    analyzer = lizard.FileAnalyzer(lizard.get_extensions([]))
    for rel, path in _walk(module_root):
        if not rel.endswith((".cpp", ".h")):
            continue
        text = _read(path)
        lines = text.splitlines()
        if rel.endswith(".cpp") and len(lines) > MAX_FILE_LINES:
            found["file " + rel] = len(lines)
        for func in analyzer.analyze_source_code(path, text).function_list:
            if func.length > MAX_FUNCTION_LINES:
                found["function %s %s" % (rel, func.name)] = func.length
            depth = nesting_depth(lines[func.start_line - 1 : func.end_line])
            if depth > MAX_NESTING:
                found["nesting %s %s" % (rel, func.name)] = depth
    return found


def logtemp_violations(module_root):
    """{"LogTemp <file>": uses}: new code logs to its system's category (cpp.md, logging)."""
    found = {}
    for rel, path in _walk(module_root):
        n = len(LOGTEMP.findall(_read(path)))
        if n:
            found["LogTemp " + rel] = n
    return found


def passed_tests(log_text):
    """Number of Jam.* automation tests that passed in an AutoTests.log."""
    return len(PASSED_JAM_TEST.findall(log_text))


def ratchet(current, baseline):
    """
    Compares violation counts. worse: new or grown since the baseline, better: shrunk or gone;
    both map the key to (baseline value, current value), 0 standing for absent.
    """
    worse, better = {}, {}
    for key in set(current) | set(baseline):
        old, new = baseline.get(key, 0), current.get(key, 0)
        if new > old:
            worse[key] = (old, new)
        elif new < old:
            better[key] = (old, new)
    return worse, better


def updated_baseline(baseline, current, passed_tests=None):
    """
    The baseline after an improvement: each violation keeps the smaller of the two counts and gone ones are
    dropped, but a new or grown violation is never adopted. The test count only goes up.
    """
    violations = {}
    for key, old in baseline["violations"].items():
        new = min(old, current.get(key, 0))
        if new:
            violations[key] = new
    tests = baseline.get("tests", 0)
    if passed_tests is not None:
        tests = max(tests, passed_tests)
    return {"violations": violations, "tests": tests}


def nesting_depth(lines):
    """
    Deepest block of a function in clang-formatted Allman code: every block opens with a "{" line one tab
    deeper than its parent, so depth = indent of the deepest "{" line minus the indent of the body's "{".
    Wrapped arguments are indented too, but never start with "{", so they do not count.
    (lizard's own nesting count is off for else-if chains and loops.)
    """
    opens = [len(line) - len(line.lstrip("\t")) for line in lines if line.strip() == "{"]
    return max(opens) - opens[0] if opens else 0


def format_offenders(source_root, clang_format="clang-format"):
    """Files under source_root that the pinned clang-format would change (relative paths)."""
    project = os.path.dirname(source_root)
    paths = [os.path.relpath(os.path.join(d, f), project).replace("\\", "/")
             for d, _, files in os.walk(source_root) for f in files if f.endswith((".cpp", ".h"))]
    offenders = set()
    for start in range(0, len(paths), 60):  # batches keep the command line under the Windows limit
        run = subprocess.run([clang_format, "--dry-run", "--style=file"] + paths[start : start + 60], cwd=project,
                             capture_output=True, text=True, encoding="utf-8", errors="ignore")
        for line in run.stderr.splitlines():
            m = FORMAT_WARNING.match(line)
            if m:
                offenders.add(m.group(1).replace("\\", "/"))
            elif "error:" in line:
                offenders.add("(clang-format error) " + line.strip())
    return sorted(offenders)


def summary(violations, passed=None, format_files=None):
    """Milestone metrics: number of violations and their total per kind, plus tests and unformatted files."""
    metrics = collections.Counter()
    for key, value in violations.items():
        kind = key.split(" ", 1)[0]
        metrics[kind + " count"] += 1
        metrics[kind + " total"] += value
    if passed is not None:
        metrics["Jam. tests passed"] = passed
    if format_files is not None:
        metrics["format files"] = format_files
    return dict(metrics)


def report(snapshot_name=None, passed=None, format_files=None):
    """Prints current milestone metrics against the newest snapshot, or writes a new snapshot."""
    os.makedirs(SNAPSHOTS, exist_ok=True)
    now = summary(current_violations(MODULE), passed, format_files)
    if snapshot_name:
        path = os.path.join(SNAPSHOTS, snapshot_name + ".json")
        with open(path, "w", encoding="utf-8", newline="\n") as h:
            json.dump(now, h, indent=1, sort_keys=True)
            h.write("\n")
        print("[check] snapshot written: " + os.path.relpath(path, os.path.dirname(SOURCE)).replace("\\", "/"))
        return
    saved = sorted(os.listdir(SNAPSHOTS), key=lambda f: os.path.getmtime(os.path.join(SNAPSHOTS, f)))
    if not saved:
        print("[check] no snapshot yet: run with --snapshot <milestone>")
        return
    with open(os.path.join(SNAPSHOTS, saved[-1]), encoding="utf-8") as h:
        before = json.load(h)
    print("[check] metrics against snapshot %s (lower is better except tests):" % saved[-1][:-5])
    for key in sorted(set(before) | set(now)):
        old, new = before.get(key, 0), now.get(key, 0)
        print("  %-24s %6d -> %-6d %s" % (key, old, new, "" if old == new else ("%+d" % (new - old))))


def current_violations(module_root):
    found = dependency_violations(module_root)
    found.update(size_violations(module_root))
    found.update(logtemp_violations(module_root))
    return found


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--tests-log", help="AutoTests.log of the run, to ratchet the Jam. test count")
    parser.add_argument("--update-baseline", action="store_true",
                        help="write shrunk violations and a higher test count into baseline.json")
    parser.add_argument("--no-format", action="store_true", help="skip the clang-format pass")
    parser.add_argument("--report", action="store_true", help="milestone trend against the newest snapshot")
    parser.add_argument("--snapshot", metavar="MILESTONE", help="save the current metrics as Tools/Quality/snapshots/<MILESTONE>.json")
    args = parser.parse_args()

    if args.report or args.snapshot:
        passed = None
        if args.tests_log and os.path.exists(args.tests_log):
            with open(args.tests_log, encoding="utf-8", errors="ignore") as h:
                passed = passed_tests(h.read())
        report(args.snapshot, passed, None if args.no_format else len(format_offenders(SOURCE)))
        return 0

    current = current_violations(MODULE)
    with open(BASELINE, encoding="utf-8") as h:
        baseline = json.load(h)
    passed = None
    if args.tests_log and os.path.exists(args.tests_log):
        with open(args.tests_log, encoding="utf-8", errors="ignore") as h:
            passed = passed_tests(h.read())

    warnings = []
    worse, better = ratchet(current, baseline["violations"])
    for key, (old, new) in sorted(worse.items()):
        warnings.append("%s: %d (baseline %d)" % (key, new, old))
    if passed is not None and passed < baseline.get("tests", 0):
        warnings.append("Jam. tests passed: %d (baseline %d)" % (passed, baseline["tests"]))
    if not args.no_format:
        for rel in format_offenders(SOURCE):
            warnings.append("format %s: run clang-format -i (pinned in Tools/Quality/requirements.txt)" % rel)

    for line in warnings:
        print("[check] WARNING " + line)
    print("[check] %d warning(s); fix each or justify it in the session report (.claude/rules/cpp.md)" % len(warnings))
    grown_tests = passed is not None and passed > baseline.get("tests", 0)
    if better or grown_tests:
        if args.update_baseline:
            with open(BASELINE, "w", encoding="utf-8", newline="\n") as h:
                json.dump(updated_baseline(baseline, current, passed), h, indent=1, sort_keys=True)
                h.write("\n")
            print("[check] baseline updated: %d improvement(s)" % (len(better) + grown_tests))
        else:
            print("[check] %d improvement(s): run with --update-baseline and commit baseline.json" %
                  (len(better) + grown_tests))
    return 0


if __name__ == "__main__":
    sys.exit(main())
