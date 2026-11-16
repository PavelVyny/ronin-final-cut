"""
KIND: offline tool (reads Claude Code session logs, writes only under _AgentLogs/).

Copies every Claude Code session of this project into the repository, so prompts and agent replies survive
context compaction, /clear, new sessions and Claude Code's own log cleanup:

    _AgentLogs/raw/<session>.jsonl                 the session log: secrets redacted, binary payloads stripped
    _AgentLogs/raw/<session>/subagents/*.jsonl     logs of the subagents that session spawned
    _AgentLogs/md/<session>.md                     readable transcript: prompts, replies, tool calls, user answers
    _AgentLogs/md/<session>__<agent>.md            the same for each subagent
    _AgentLogs/INDEX.md                            one line per session

Sessions are found by folder: Claude Code keeps them in ~/.claude/projects/<slug>/, where <slug> is the working
directory with every non-alphanumeric character replaced by "-". Every slug starting with this repository's slug
is taken (git worktrees next to the repository included). Sessions run from another folder are listed in
Tools/Devlog/sessions.json as {"slug": ..., "session": ..., "note": ...}. An entry with "until" (a UTC stamp) is
frozen: nothing logged after that moment is exported.

    python Tools/Devlog/export_sessions.py [--projects-dir DIR] [--out DIR] [--results-chars N] [--quiet]

The Markdown is for people; review it before publishing (it still holds file paths and whatever was typed).
Tests: Tools/tests/test_export_sessions.py
"""
import argparse
import json
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.normpath(os.path.join(HERE, "..", ".."))
EXTRA = os.path.join(HERE, "sessions.json")
# Jam window in UTC (12 Nov 00:00 ET to 15 Nov 20:59 ET): the index marks each session against it.
JAM_START = "2026-11-12T05:00:00"
JAM_END = "2026-11-16T01:59:00"

# Bookkeeping entries with no conversation in them (some hold account ids).
KEEP_TYPES = {"user", "assistant", "system", "attachment", "queue-operation", "ai-title"}
# Group 1, when present, is the prefix that stays (the variable name, "Bearer ").
SECRETS = [
    re.compile(r"()\bfal_sk_[A-Za-z0-9]{8,}(?::[A-Za-z0-9]{8,})?"),
    re.compile(r"()\bsk-[A-Za-z0-9_-]{20,}"),
    re.compile(r"()\bsk_[A-Za-z0-9]{24,}"),
    re.compile(r"()\btsk_[A-Za-z0-9_-]{20,}"),
    re.compile(r"()\bxox[abprs]-[A-Za-z0-9-]{10,}"),
    re.compile(r"()\bgh[pousr]_[A-Za-z0-9]{30,}"),
    re.compile(r"()\bAIza[0-9A-Za-z_-]{35}"),
    re.compile(r"(Bearer )[A-Za-z0-9._-]{20,}"),
    re.compile(r"((?:KEY|TOKEN|SECRET|PASSWORD)[\"']?\s*[=:]\s*[\"']?)[^\s\"'\\,<]{8,}"),
]
EMAIL = re.compile(r"[A-Za-z0-9._%+-]+@[A-Za-z0-9-]+\.[A-Za-z0-9.-]*[A-Za-z]")
BASE64 = re.compile(r"[A-Za-z0-9+/=\r\n]+")
REMINDER = re.compile(r"<system-reminder>.*?</system-reminder>", re.S)
COMMAND = re.compile(r"<command-name>(.*?)</command-name>", re.S)
# A finished subagent's report comes back through the same queue as the user's mid-turn messages.
AGENT_REPORT = "<agent-message"
MAX_INLINE = 300


def slug_of(path):
    """Claude Code's folder name for a working directory."""
    return re.sub(r"[^A-Za-z0-9]", "-", os.path.normpath(path))


def redact(text):
    """text with API keys, tokens and e-mail addresses replaced by placeholders."""
    for pattern in SECRETS:
        text = pattern.sub(r"\1<redacted-secret>", text)
    return EMAIL.sub("<redacted-email>", text)


def scrub(value, key=None):
    """A JSON value with secrets redacted, binary payloads and thinking signatures replaced by their size."""
    if isinstance(value, dict):
        return {k: scrub(v, k) for k, v in value.items()}
    if isinstance(value, list):
        return [scrub(v, key) for v in value]
    if isinstance(value, str):
        if key == "signature" or (len(value) > 2000 and BASE64.fullmatch(value)):
            return "<stripped %d chars>" % len(value)
        return redact(value)
    return value


def scrub_line(line):
    """One JSONL line ready for the repository, or None for a bookkeeping entry or a broken line."""
    try:
        entry = json.loads(line)
    except ValueError:
        return None
    if entry.get("type") not in KEEP_TYPES:
        return None
    return json.dumps(scrub(entry), ensure_ascii=False, separators=(",", ":"))


def _short(text, limit=MAX_INLINE):
    text = " ".join(str(text).split())
    return text if len(text) <= limit else text[:limit] + " ..."


def _flat(content):
    """Text of a tool_result content, which is a string or a list of blocks."""
    if isinstance(content, str):
        return content
    if isinstance(content, list):
        return "\n".join(b.get("text", "") for b in content if isinstance(b, dict) and b.get("type") == "text")
    return ""


def tool_call_md(name, args):
    """Markdown for one tool call: the inputs a reader needs, not the whole payload."""
    if name == "Agent":
        return "> **Subagent** `%s`: %s\n\n<details><summary>Prompt</summary>\n\n%s\n\n</details>" % (
            args.get("subagent_type", "general-purpose"), args.get("description", ""), args.get("prompt", ""))
    if name == "AskUserQuestion":
        lines = ["> **Question to the user**"]
        for q in args.get("questions", []):
            lines.append("> - %s" % q.get("question", ""))
            for option in q.get("options", []):
                lines.append(">   - %s: %s" % (option.get("label", ""), _short(option.get("description", ""))))
        return "\n".join(lines)
    if name == "Bash":
        detail = "%s: `%s`" % (args.get("description", ""), _short(args.get("command", "")))
    elif "file_path" in args:
        detail = "`%s`" % args["file_path"]
    else:
        detail = "`%s`" % _short(json.dumps(args, ensure_ascii=False))
    return "> **Tool** `%s` %s" % (name, detail)


def user_text_md(text):
    """A typed user message without the harness reminders; slash commands shown as the command itself."""
    command = COMMAND.search(text)
    if command:
        return command.group(1).strip()
    if text.lstrip().startswith(("<local-command-", "<task-notification>")):
        return ""
    return REMINDER.sub("", text).strip()


def typed_prompts(entries):
    """Texts of the user's ordinary messages (string content), to tell them from messages queued mid-turn."""
    return {user_text_md(c) for c in ((e.get("message") or {}).get("content") for e in entries if e.get("type") == "user")
            if isinstance(c, str)}


def queued_prompt(entry, typed):
    """Text of a message the user sent while the agent was working, or "" when the entry is not one.

    Such a message is logged only as a queue entry; one typed while the agent was idle also gets a user entry,
    which is rendered instead.
    """
    if entry.get("type") != "queue-operation" or entry.get("operation") != "enqueue":
        return ""
    text = user_text_md(str(entry.get("content") or ""))
    return "" if text in typed else text


def render(entries, title, results_chars=0):
    """Markdown transcript of a session's entries (already scrubbed dicts, in file order)."""
    out = ["# %s" % title, ""]
    calls = {}
    typed = typed_prompts(entries)
    for e in entries:
        kind = e.get("type")
        stamp = e.get("timestamp", "")[:16].replace("T", " ")
        if kind == "system" and e.get("subtype") == "compact_boundary":
            out += ["---", "*Context compacted here (%s UTC). The log above is complete; the summary follows.*" % stamp, ""]
            continue
        queued = queued_prompt(e, typed)
        if queued:
            who = "Subagent report" if queued.startswith(AGENT_REPORT) else "User, sent while Claude was working"
            out += ["## %s (%s UTC)" % (who, stamp), "", queued, ""]
            continue
        if kind not in ("user", "assistant") or e.get("isMeta"):
            continue
        content = (e.get("message") or {}).get("content")
        blocks = [{"type": "text", "text": content}] if isinstance(content, str) else (content or [])
        for b in blocks:
            if not isinstance(b, dict):
                continue
            btype = b.get("type")
            if btype == "text" and kind == "user":
                text = user_text_md(b.get("text", ""))
                if e.get("isCompactSummary"):
                    out += ["<details><summary>Compaction summary</summary>", "", text, "", "</details>", ""]
                elif text:
                    out += ["## User (%s UTC)" % stamp, "", text, ""]
            elif btype == "text" and b.get("text", "").strip():
                out += ["## Claude (%s UTC)" % stamp, "", b["text"].strip(), ""]
            elif btype == "tool_use":
                calls[b.get("id")] = b.get("name")
                out += [tool_call_md(b.get("name", "?"), b.get("input") or {}), ""]
            elif btype == "tool_result":
                name = calls.get(b.get("tool_use_id"))
                text = _flat(b.get("content"))
                if name == "AskUserQuestion":
                    out += ["## User answers (%s UTC)" % stamp, "", text.strip(), ""]
                elif results_chars and text.strip():
                    out += ["```", text.strip()[:results_chars], "```", ""]
    return "\n".join(out).rstrip() + "\n"


def summary_of(entries):
    """Index facts of a session: first and last timestamp, title, prompt and reply counts."""
    stamps = [e["timestamp"][:19] for e in entries if e.get("timestamp")]
    titles = [e.get("aiTitle") for e in entries if e.get("type") == "ai-title" and e.get("aiTitle")]
    typed = typed_prompts(entries)
    prompts = sum(1 for e in entries if queued_prompt(e, typed) and not queued_prompt(e, typed).startswith(AGENT_REPORT))
    replies = 0
    for e in entries:
        content = (e.get("message") or {}).get("content")
        blocks = [{"type": "text", "text": content}] if isinstance(content, str) else (content or [])
        texts = [b for b in blocks if isinstance(b, dict) and b.get("type") == "text"]
        if e.get("type") == "user" and not e.get("isMeta") and not e.get("isCompactSummary"):
            prompts += sum(1 for b in texts if user_text_md(b.get("text", "")))
        elif e.get("type") == "assistant":
            replies += len(texts)
    return {"start": min(stamps) if stamps else "", "end": max(stamps) if stamps else "",
            "title": titles[-1] if titles else "", "prompts": prompts, "replies": replies}


def phase_of(start, end):
    """Where a session sits against the jam window."""
    if not start:
        return "empty"
    if end < JAM_START:
        return "pre-jam"
    if start >= JAM_END:
        return "after"
    return "jam" if start >= JAM_START else "spans the start"


def find_sessions(projects_dir, root, extra):
    """[(slug, session id, note, until)] of every session that belongs to the project."""
    prefix = slug_of(root)
    found = []
    if os.path.isdir(projects_dir):
        for slug in sorted(os.listdir(projects_dir)):
            folder = os.path.join(projects_dir, slug)
            if slug.startswith(prefix) and os.path.isdir(folder):
                found += [(slug, f[:-6], "", "") for f in sorted(os.listdir(folder)) if f.endswith(".jsonl")]
    found += [(x["slug"], x["session"], x.get("note", ""), x.get("until", "")) for x in extra]
    return found


def _export_log(src, dst_raw, dst_md, title, results_chars, until=""):
    """Scrubs one log into dst_raw and renders dst_md, stopping at the first entry after until; returns the entries."""
    entries = []
    os.makedirs(os.path.dirname(dst_raw), exist_ok=True)
    with open(src, encoding="utf-8", errors="replace") as h, open(dst_raw, "w", encoding="utf-8", newline="\n") as o:
        for line in h:
            clean = scrub_line(line)
            if clean is not None:
                entry = json.loads(clean)
                if until and entry.get("timestamp", "") > until:
                    break
                o.write(clean + "\n")
                entries.append(entry)
    os.makedirs(os.path.dirname(dst_md), exist_ok=True)
    with open(dst_md, "w", encoding="utf-8", newline="\n") as o:
        o.write(render(entries, title, results_chars))
    return entries


def export(projects_dir, out, root, extra, results_chars=0):
    """Exports every session; returns the index rows."""
    rows = []
    for slug, session, note, until in find_sessions(projects_dir, root, extra):
        src = os.path.join(projects_dir, slug, session + ".jsonl")
        if not os.path.exists(src):
            print("[devlog] WARNING missing session log: " + src)
            continue
        entries = _export_log(src, os.path.join(out, "raw", session + ".jsonl"),
                              os.path.join(out, "md", session + ".md"), "Session " + session, results_chars, until)
        info = summary_of(entries)
        agents_dir = os.path.join(projects_dir, slug, session, "subagents")
        agents = sorted(f[:-6] for f in os.listdir(agents_dir) if f.endswith(".jsonl")) if os.path.isdir(agents_dir) else []
        for agent in agents:
            _export_log(os.path.join(agents_dir, agent + ".jsonl"),
                        os.path.join(out, "raw", session, "subagents", agent + ".jsonl"),
                        os.path.join(out, "md", "%s__%s.md" % (session, agent)),
                        "Subagent %s of session %s" % (agent, session), results_chars, until)
        info.update(session=session, note=note, agents=len(agents), phase=phase_of(info["start"], info["end"]))
        rows.append(info)
    rows.sort(key=lambda r: r["start"])
    lines = ["# Agent session logs", "",
             "Written by `Tools/Devlog/export_sessions.py`. Times are UTC; the jam window is %s to %s." % (JAM_START, JAM_END), "",
             "| Start | End | Phase | Prompts | Replies | Subagents | Transcript | Title |", "|---|---|---|---|---|---|---|---|"]
    for r in rows:
        lines.append("| %s | %s | %s | %d | %d | %d | [%s](md/%s.md) | %s |" % (
            r["start"].replace("T", " "), r["end"].replace("T", " "), r["phase"], r["prompts"], r["replies"], r["agents"],
            r["session"][:8], r["session"], " ".join(x for x in (r["title"], r["note"]) if x).replace("|", "/")))
    os.makedirs(out, exist_ok=True)
    with open(os.path.join(out, "INDEX.md"), "w", encoding="utf-8", newline="\n") as o:
        o.write("\n".join(lines) + "\n")
    return rows


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--projects-dir", default=os.path.join(os.path.expanduser("~"), ".claude", "projects"))
    parser.add_argument("--out", default=os.path.join(ROOT, "_AgentLogs"))
    parser.add_argument("--results-chars", type=int, default=0, help="characters of each tool result kept in the Markdown")
    parser.add_argument("--quiet", action="store_true")
    args = parser.parse_args()
    extra = []
    if os.path.exists(EXTRA):
        with open(EXTRA, encoding="utf-8") as h:
            extra = json.load(h)
    rows = export(args.projects_dir, args.out, ROOT, extra, args.results_chars)
    if not args.quiet:
        print("[devlog] %d session(s), %d subagent log(s) -> %s" % (len(rows), sum(r["agents"] for r in rows), args.out))
    return 0


if __name__ == "__main__":
    sys.exit(main())
