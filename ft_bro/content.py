"""Join engine facts to spec prose by fn:id.

The engine emits {"fn":..,"id":..,"status":..} and nothing else. why / fix /
kw / ref come from cases.json, generated from _dev/plan/rank00/libft-01-cases.md (A14), so
rewording an explanation rebuilds nothing.
"""

import json

from . import paths

_CACHE = {}


def _load(name, generator):
    if name not in _CACHE:
        path = paths.ROOT / "ft_bro" / "data" / f"{name}.json"
        if not path.is_file():
            raise SystemExit(
                f"ft_bro/data/{name}.json missing - run: "
                f"python3 ft_bro/data/{generator}.py")
        _CACHE[name] = json.loads(path.read_text())
    return _CACHE[name]


def load():
    return _load("cases", "gen_cases")


def concepts():
    return _load("concepts", "gen_concepts")["concepts"]


def concept(slug):
    return concepts().get(slug, {})


def defense_bank():
    return _load("concepts", "gen_concepts")["defense"]


def roadmap():
    """{fn: {"level":, "prereqs":[...], "order":}} - _dev/plan/rank00/libft-02-learning.md's
    progressive track. Prerequisites only; level/part/tier for rendering
    come from cases.json (03_ORCHESTRATOR.md)."""
    return _load("roadmap", "gen_roadmap")["functions"]


def case(fn, cid):
    return load()["cases"].get(f"{fn}:{cid}", {})


def function(fn):
    return load()["functions"].get(fn, {})


def annotate_missing(records, info):
    """Say WHICH kind of missing. See build.prepare()."""
    broken = set(info.get("not_compiling") or ())
    blocked = info.get("blocked") or {}
    errors = info.get("compile_errors") or {}
    for r in records:
        if r.get("status") != "MISSING":
            continue
        if r["fn"] in blocked:
            r["missing_reason"] = "blocked"
            r["blocked_on"] = blocked[r["fn"]]
            need = ", ".join(blocked[r["fn"]])
            r["msg"] = (f"{r['fn']} needs {need}, which is not written yet - "
                        f"write that first and this unblocks itself")
        elif r["fn"] in broken:
            r["missing_reason"] = "does not compile"
            err = errors.get(r["fn"])
            if err:
                r["compile_error"] = err
                r["msg"] = f"{r['fn']}.c exists but did not compile: {err}"
            else:
                r["msg"] = f"{r['fn']}.c exists but did not compile"
        else:
            r["missing_reason"] = "not written"
            r["msg"] = f"{r['fn']} is not written yet"
    return records


def join(record):
    """Engine record + spec prose -> what the renderer draws."""
    meta = case(record["fn"], record["id"])
    merged = dict(record)
    merged["why"] = meta.get("why", "")
    merged["fix"] = meta.get("fix", "")
    merged["note"] = meta.get("note", "")
    merged["kw"] = meta.get("kw", [])
    merged["ref"] = meta.get("ref", "")
    merged["input"] = meta.get("input", "")
    merged["group"] = meta.get("group", "")
    return merged
