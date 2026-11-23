#!/usr/bin/env python3
"""Validate and roll the Y2KBox active-session summary at Claude Code Stop."""
from __future__ import annotations

import json
import os
import re
import sys
from pathlib import Path

ROOT = Path(os.environ.get("CLAUDE_PROJECT_DIR", Path(__file__).resolve().parent.parent))
WORKLOG = ROOT / "WORKLOG.md"
ACTIVE = ROOT / "WORKLOAD_ACTIVE_SESSION.md"
ACTIVE_HEADING = re.compile(r"^### Session\s+(\d+)\b")
WORKLOG_HEADING = re.compile(r"^#{2,3}\s+Session\s+(\d+)\b", re.IGNORECASE)
TOP_HEADING = re.compile(r"^## ")

def session_ids(path: Path, *, worklog: bool = False) -> list[int]:
    if not path.is_file():
        return []
    heading = WORKLOG_HEADING if worklog else ACTIVE_HEADING
    found: list[int] = []
    for line in path.read_text(encoding="utf-8").splitlines():
        match = heading.match(line)
        # Owner-validation notes may be appended later but belong to an earlier session.
        if match and not (worklog and "owner validation" in line.lower()):
            value = int(match.group(1))
            if not found or found[-1] != value:
                found.append(value)
    return found

def prune_oldest_active_entries() -> None:
    lines = ACTIVE.read_text(encoding="utf-8").splitlines(keepends=True)
    starts = [i for i, line in enumerate(lines) if ACTIVE_HEADING.match(line)]
    if len(starts) <= 10:
        return

    remove_count = len(starts) - 10
    remove_ranges: list[tuple[int, int]] = []
    for start in starts[:remove_count]:
        end = len(lines)
        for i in range(start + 1, len(lines)):
            if ACTIVE_HEADING.match(lines[i]) or TOP_HEADING.match(lines[i]):
                end = i
                break
        remove_ranges.append((start, end))

    remove_lines = {i for start, end in remove_ranges for i in range(start, end)}
    ACTIVE.write_text("".join(line for i, line in enumerate(lines) if i not in remove_lines),
                      encoding="utf-8")

def stop(message: str) -> None:
    # Claude Code Stop hooks understand this decision and keep the session open.
    print(json.dumps({"decision": "block", "reason": message}))
    raise SystemExit(0)

def main() -> int:
    if not WORKLOG.is_file() or not ACTIVE.is_file():
        stop("Session handoff incomplete: WORKLOG.md or WORKLOAD_ACTIVE_SESSION.md is missing.")

    worklog_ids = session_ids(WORKLOG, worklog=True)
    if not worklog_ids:
        stop("No session headings found in WORKLOG.md; check the session heading format.")

    # If the agent added a new condensed entry, automatically roll the window forward.
    prune_oldest_active_entries()
    active_ids = session_ids(ACTIVE)
    expected = worklog_ids[-10:]

    if active_ids != expected:
        latest = worklog_ids[-1]
        active_latest = active_ids[-1] if active_ids else "none"
        stop(
            "Update WORKLOAD_ACTIVE_SESSION.md before ending this session. "
            f"WORKLOG.md now contains session(s) through {latest}, but the active summary "
            f"contains session(s) through {active_latest}. Add a concise entry for every "
            "new worklog session. The Stop hook automatically removes entries older than "
            "the latest ten; WORKLOG.md remains the permanent append-only history."
        )
    return 0

if __name__ == "__main__":
    sys.exit(main())
