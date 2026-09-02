"""Join engine facts to spec prose by fn:id.

The engine emits {"fn":..,"id":..,"status":..} and nothing else. why / fix /
kw / ref come from <pack>/data/cases.json, generated from that pack's authored
case spec (A14), so rewording an explanation rebuilds nothing.

Everything here is scoped to a pack (bro42/packs/__init__.py): the cache key
and the data directory both come from it, so two packs' data can be loaded in
the same process (e.g. `bro --serve` re-running after a `--pack` switch)
without one clobbering the other's cache entry.
"""

import json

_CACHE = {}


def _load(pack, name, generator):
    key = (pack.id, name)
    if key not in _CACHE:
        path = pack.data_dir() / f"{name}.json"
        if not path.is_file():
            raise SystemExit(
                f"{path} missing - run: "
                f"python3 bro42/data/{generator}.py --pack {pack.id}")
        _CACHE[key] = json.loads(path.read_text())
    return _CACHE[key]


def load(pack):
    return _load(pack, "cases", "gen_cases")


def concepts(pack):
    return _load(pack, "concepts", "gen_concepts")["concepts"]


def concept(pack, slug):
    return concepts(pack).get(slug, {})


def defense_bank(pack):
    return _load(pack, "concepts", "gen_concepts")["defense"]


def roadmap(pack):
    """{fn: {"level":, "prereqs":[...], "order":}} - the pack's progressive
    track. Prerequisites only; level/part/tier for rendering come from
    cases.json (03_ORCHESTRATOR.md)."""
    return _load(pack, "roadmap", "gen_roadmap")["functions"]


def case(pack, fn, cid):
    return load(pack)["cases"].get(f"{fn}:{cid}", {})


def function(pack, fn):
    return load(pack)["functions"].get(fn, {})


def annotate_missing(pack, records, info):
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


def join(pack, record):
    """Engine record + spec prose -> what the renderer draws."""
    meta = case(pack, record["fn"], record["id"])
    merged = dict(record)
    merged["why"] = meta.get("why", "")
    merged["fix"] = meta.get("fix", "")
    merged["note"] = meta.get("note", "")
    merged["kw"] = meta.get("kw", [])
    merged["ref"] = meta.get("ref", "")
    merged["input"] = meta.get("input", "")
    merged["group"] = meta.get("group", "")
    return merged
