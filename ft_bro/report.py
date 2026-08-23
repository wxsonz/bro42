"""Assemble report.json and inline it into a self-contained report.html.

Decision A2: the data is inlined at generation, so the page opens by
double-click, works from file://, and can be handed to a peer as one file.

The previous SPEC_FRONTEND revision specified a "Static Standalone Mode" where
index.html fetches report.json - which cannot work, because fetch() against a
file:// URL is CORS-blocked in every current browser. Inlining is what makes
the offline promise real.
"""

import json
import platform
import shutil
from datetime import datetime, timezone
from pathlib import Path

from . import VERSION, content, history, paths

SCHEMA = 1
UNSCORED = {"UB", "MISSING", "SKIP"}
BAD = {"KO", "SIGSEGV", "SIGBUS", "SIGABRT", "TIMEOUT", "LEAK"}
WEB = Path(__file__).resolve().parent.parent / "web"


def build_report(records, target, checks=None, hist=None, hist_delta=None):
    cases = [r for r in records if r.get("kind") != "macro"]
    scored = [r for r in cases if r["status"] not in UNSCORED]
    funcs = content.load()["functions"]
    road = content.roadmap()

    suites = {}
    for r in cases:
        s = suites.setdefault(r["fn"], {
            "func_name": r["fn"],
            "part": r.get("part"), "level": r.get("level"), "tier": r.get("tier"),
            "prototype": funcs.get(r["fn"], {}).get("prototype", ""),
            "oracle": funcs.get(r["fn"], {}).get("oracle", ""),
            "present": True, "cases": [],
        })
        if r["status"] == "MISSING":
            s["present"] = False
        s["cases"].append(r)
    for s in suites.values():
        s["cases"].sort(key=lambda c: c["id"])
        sc = [c for c in s["cases"] if c["status"] not in UNSCORED]
        s["test_count"] = len(sc)
        s["pass_count"] = sum(1 for c in sc if c["status"] == "OK")
        # SPEC_FRONTEND #2: the roadmap draws prerequisite edges so a red node
        # whose prerequisite is also red sits visibly downstream of it. The
        # engine never sees this - it is the progressive track from
        # SPEC_LEARNING.md, joined on here the same way why/fix/kw are.
        s["prereqs"] = road.get(s["func_name"], {}).get("prereqs", [])

    counts = {}
    for r in cases:
        counts[r["status"]] = counts.get(r["status"], 0) + 1

    return {
        "schema": SCHEMA,
        "generated": datetime.now(timezone.utc).isoformat(timespec="seconds"),
        "target": str(target),
        "bro_version": VERSION,
        "platform": {
            "os": platform.system().lower(),
            "norminette": bool(shutil.which("norminette")),
        },
        "summary": {
            "total_cases": len(scored),
            "passed": sum(1 for r in scored if r["status"] == "OK"),
            "failed": sum(1 for r in scored if r["status"] in BAD),
            "present_funcs": sum(1 for s in suites.values() if s["present"]),
            "total_funcs": len(suites),
            "counts": counts,
        },
        "delta": hist_delta,
        "history": hist or [],
        "concepts": content.concepts(),
        "defense": content.defense_bank(),
        "macro": checks or [],
        "suites": sorted(suites.values(),
                         key=lambda s: funcs.get(s["func_name"], {}).get("order", 0)),
    }


def write(records, target, checks=None, hist_delta=None):
    cache = paths.cache_dir(target)
    hist = history.read(target)[-60:]
    report = build_report(records, target, checks, hist, hist_delta)

    (cache / "report.json").write_text(json.dumps(report, indent=1) + "\n")

    html = (WEB / "template.html").read_text()
    html = html.replace("/*__STYLE__*/", (WEB / "style.css").read_text())
    html = html.replace("/*__APP__*/", (WEB / "app.js").read_text())
    # The data goes in as JSON inside a <script type="application/json">, so
    # nothing in it can be parsed as markup or as code.
    payload = json.dumps(report).replace("</", "<\\/")
    html = html.replace("__DATA__", payload)
    out = cache / "report.html"
    out.write_text(html)
    return out
