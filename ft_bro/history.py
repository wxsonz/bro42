"""Run history and the since-last-run delta (decision A10).

This is the feature that makes it a companion rather than a scoreboard. One
append-only JSONL line per run, in the cache - never in the student's repo -
and the delta is a diff of the last two lines.

`broke` leads the display for a reason: a regression the student did not notice
is the single most valuable thing this tool can tell them.
"""

import json
from datetime import datetime, timezone

from . import paths

UNSCORED = {"UB", "MISSING", "SKIP"}


def _path(target):
    return paths.cache_dir(target) / "history.jsonl"


def summarise(records, macro=None):
    cases = [r for r in records if r.get("kind") != "macro"]
    scored = [r for r in cases if r["status"] not in UNSCORED]
    fails = sorted(f"{r['fn']}:{r['id']}" for r in scored if r["status"] != "OK")
    present = sorted({r["fn"] for r in cases if r["status"] != "MISSING"})
    counts = {}
    for r in cases:
        counts[r["status"]] = counts.get(r["status"], 0) + 1
    entry = {
        "ts": datetime.now(timezone.utc).isoformat(timespec="seconds"),
        "pass": sum(1 for r in scored if r["status"] == "OK"),
        "total": len(scored),
        "counts": counts,
        "fails": fails,
        "present": present,
    }
    if macro is not None:
        entry["macro"] = {
            "fail": sum(1 for c in macro if c["status"] == "FAIL"),
            "warn": sum(1 for c in macro if c["status"] == "WARN"),
            "skip": sum(1 for c in macro if c["status"] == "SKIP"),
        }
    return entry


def read(target):
    path = _path(target)
    if not path.is_file():
        return []
    out = []
    for line in path.read_text().splitlines():
        line = line.strip()
        if line:
            try:
                out.append(json.loads(line))
            except json.JSONDecodeError:
                continue
    return out


def append(target, entry):
    with _path(target).open("a") as fh:
        fh.write(json.dumps(entry, sort_keys=True) + "\n")


def streak(entries):
    """Consecutive calendar days ending today with at least one run."""
    days = sorted({e["ts"][:10] for e in entries}, reverse=True)
    if not days:
        return 0
    from datetime import date, timedelta
    today = datetime.now(timezone.utc).date()
    if date.fromisoformat(days[0]) < today - timedelta(days=1):
        return 0
    n, cursor = 0, date.fromisoformat(days[0])
    for d in days:
        if date.fromisoformat(d) == cursor:
            n += 1
            cursor -= timedelta(days=1)
        else:
            break
    return n


def delta(previous, current, entries):
    if not previous:
        return None
    was, now = set(previous["fails"]), set(current["fails"])
    return {
        "since": previous["ts"],
        "fixed": sorted(was - now),
        "broke": sorted(now - was),
        "new_funcs": sorted(set(current["present"]) - set(previous["present"])),
        "gone_funcs": sorted(set(previous["present"]) - set(current["present"])),
        "from": (previous["pass"], previous["total"]),
        "to": (current["pass"], current["total"]),
        "streak": streak(entries),
    }
