"""Tests of Tools/Devlog/export_sessions.py: redaction, rendering and session discovery."""
import json
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "Devlog"))

import export_sessions as es


def _user(text, **extra):
    return dict({"type": "user", "timestamp": "2026-11-12T06:00:00.000Z", "message": {"role": "user", "content": text}}, **extra)


def _assistant(*blocks):
    return {"type": "assistant", "timestamp": "2026-11-12T06:01:00.000Z", "message": {"role": "assistant", "content": list(blocks)}}


def test_slug_replaces_every_non_alphanumeric():
    assert es.slug_of("F:\\Unreal_Engine\\UnrealProjects\\MegaJam2026") == "F--Unreal-Engine-UnrealProjects-MegaJam2026"


def test_redact_hides_keys_and_emails_and_keeps_the_variable_name():
    text = es.redact("OPENAI_API_KEY=abc123def456 mail someone@example.com token sk-" + "a" * 30)
    assert "abc123def456" not in text and "someone@example.com" not in text and "aaaa" not in text
    assert "OPENAI_API_KEY=<redacted-secret>" in text


def test_redact_hides_a_fal_key_pasted_without_a_variable_name():
    key = "fal_sk_" + "0a1b2c3d" * 4 + ":" + "9f8e7d6c" * 4
    text = es.redact("use the fal key " + key + "\nthen Meshy")
    assert text == "use the fal key <redacted-secret>\nthen Meshy"


def test_scrub_strips_binary_payloads_and_signatures():
    entry = {"type": "user", "message": {"content": [{"type": "image", "source": {"data": "QUJD" * 1000}}]},
             "x": {"signature": "abc"}}
    clean = es.scrub(entry)
    assert clean["message"]["content"][0]["source"]["data"] == "<stripped 4000 chars>"
    assert clean["x"]["signature"] == "<stripped 3 chars>"


def test_scrub_line_drops_bookkeeping_and_broken_lines():
    assert es.scrub_line(json.dumps({"type": "bridge-session", "ownerAccountUuid": "1"})) is None
    assert es.scrub_line("{not json") is None
    assert json.loads(es.scrub_line(json.dumps(_user("hi"))))["message"]["content"] == "hi"


def test_render_shows_prompts_replies_and_tool_calls_but_not_reminders_or_thinking():
    entries = [
        _user("build it<system-reminder>private rules</system-reminder>"),
        _assistant({"type": "thinking", "thinking": "secret plan"}, {"type": "text", "text": "Building."},
                   {"type": "tool_use", "id": "t1", "name": "Bash", "input": {"description": "Run build", "command": "make"}}),
        _user([{"type": "tool_result", "tool_use_id": "t1", "content": "ok output"}]),
    ]
    md = es.render(entries, "Session x")
    assert "## User (2026-11-12 06:00 UTC)" in md and "build it" in md and "Building." in md
    assert "> **Tool** `Bash` Run build: `make`" in md
    assert "private rules" not in md and "secret plan" not in md and "ok output" not in md
    assert "ok output" in es.render(entries, "Session x", results_chars=100)


def test_render_keeps_user_answers_and_subagent_prompts():
    entries = [
        _assistant({"type": "tool_use", "id": "q1", "name": "AskUserQuestion",
                    "input": {"questions": [{"question": "Which one?", "options": [{"label": "A", "description": "first"}]}]}},
                   {"type": "tool_use", "id": "a1", "name": "Agent",
                    "input": {"subagent_type": "Explore", "description": "Find files", "prompt": "List every script."}}),
        _user([{"type": "tool_result", "tool_use_id": "q1", "content": [{"type": "text", "text": "Which one?=A"}]}]),
    ]
    md = es.render(entries, "Session x")
    assert "> - Which one?" in md and "## User answers" in md and "Which one?=A" in md
    assert "**Subagent** `Explore`: Find files" in md and "List every script." in md


def test_render_marks_compaction_and_folds_the_summary():
    entries = [
        {"type": "system", "subtype": "compact_boundary", "timestamp": "2026-11-12T07:00:00.000Z"},
        _user("This session is being continued", isCompactSummary=True),
        _user("<command-name>/clear</command-name>"),
        _user("<local-command-stdout></local-command-stdout>"),
    ]
    md = es.render(entries, "Session x")
    assert "Context compacted here" in md and "<details><summary>Compaction summary</summary>" in md
    assert md.count("## User (") == 1 and "/clear" in md


def test_message_sent_mid_turn_is_rendered_once():
    def queued(text, operation="enqueue"):
        return {"type": "queue-operation", "operation": operation, "timestamp": "2026-11-12T06:05:00.000Z", "content": text}

    entries = [queued("wait with the build"), queued("wait with the build", "remove"), queued("typed while idle"),
               _user("typed while idle"), queued("<task-notification>done</task-notification>"),
               queued('<agent-message from="a1">found 3 files</agent-message>')]
    md = es.render(entries, "Session x")
    assert "## Subagent report (2026-11-12 06:05 UTC)" in md and "found 3 files" in md
    assert md.count("wait with the build") == 1 and "sent while Claude was working (2026-11-12 06:05 UTC)" in md
    assert md.count("typed while idle") == 1 and "task-notification" not in md
    assert es.summary_of(entries)["prompts"] == 2


def test_phase_against_the_jam_window():
    assert es.phase_of("2026-11-11T22:00:00", "2026-11-12T04:00:00") == "pre-jam"
    assert es.phase_of("2026-11-11T22:00:00", "2026-11-12T06:00:00") == "spans the start"
    assert es.phase_of("2026-11-13T10:00:00", "2026-11-13T12:00:00") == "jam"
    assert es.phase_of("", "") == "empty"


def test_export_takes_project_sessions_worktrees_extras_and_subagents(tmp_path):
    projects = tmp_path / "projects"
    root = str(tmp_path / "Jam")
    slug = es.slug_of(root)
    for folder, session in ((slug, "s1"), (slug + "-wt-sim", "s2"), ("Other", "s3"), ("Other", "s4")):
        (projects / folder).mkdir(parents=True, exist_ok=True)
        (projects / folder / (session + ".jsonl")).write_text(json.dumps(_user("prompt " + session)) + "\n", encoding="utf-8")
    agents = projects / slug / "s1" / "subagents"
    agents.mkdir(parents=True)
    (agents / "agent-a.jsonl").write_text(json.dumps(_user("subagent prompt KEY_TOKEN=abcdefgh1234")) + "\n", encoding="utf-8")
    out = tmp_path / "out"

    rows = es.export(str(projects), str(out), root, [{"slug": "Other", "session": "s3", "note": "prep"}])

    assert sorted(r["session"] for r in rows) == ["s1", "s2", "s3"]
    assert [r["agents"] for r in rows if r["session"] == "s1"] == [1]
    assert (out / "raw" / "s1" / "subagents" / "agent-a.jsonl").exists()
    agent_md = (out / "md" / "s1__agent-a.md").read_text(encoding="utf-8")
    assert "subagent prompt" in agent_md and "abcdefgh1234" not in agent_md
    index = (out / "INDEX.md").read_text(encoding="utf-8")
    assert "md/s3.md" in index and "prep" in index and "s4" not in index


def test_export_stops_a_frozen_session_at_its_until_stamp(tmp_path):
    projects = tmp_path / "projects"
    (projects / "Other").mkdir(parents=True)
    late = dict(_user("late brainstorm"), timestamp="2026-11-12T03:00:00.000Z")
    log = [_user("early prep"), {"type": "ai-title", "aiTitle": "late title"}, late, _user("after the gap")]
    log[0]["timestamp"] = "2026-11-12T00:10:00.000Z"
    (projects / "Other" / "s3.jsonl").write_text("\n".join(json.dumps(e) for e in log) + "\n", encoding="utf-8")
    out = tmp_path / "out"

    rows = es.export(str(projects), str(out), str(tmp_path / "Jam"),
                     [{"slug": "Other", "session": "s3", "until": "2026-11-12T00:23:28"}])

    raw = (out / "raw" / "s3.jsonl").read_text(encoding="utf-8")
    assert "early prep" in raw and "late title" in raw and "late brainstorm" not in raw and "after the gap" not in raw
    assert rows[0]["end"] == "2026-11-12T00:10:00" and rows[0]["phase"] == "pre-jam"
