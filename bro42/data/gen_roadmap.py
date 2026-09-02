#!/usr/bin/env python3
"""Generate bro42/data/roadmap.json from the progressive track in
_dev/plan/rank00/libft-02-learning.md.

Same principle as gen_cases.py / gen_concepts.py (decision A14): the order to
write functions in is authored once, as prose and ASCII-art in
plan/rank00/libft-02-learning.md, and this turns it into the sidecar `bro next` walks.

NOTE — level / part / tier for a function already come from cases.json
(generated from libft-01-cases.md's `**Part** N . **Level** N . **Tier** TN`
line). This file carries only the one thing libft-01-cases.md cannot express:
*prerequisites* — which other functions a student should have already
written. `level` is duplicated here too (see below), purely so `bro next`
does not have to load two sidecars to explain itself, but cases.json remains
the source of truth for it and this generator cross-checks against it.

Grammar (see "Progressive track" in plan/rank00/libft-02-learning.md):

    Level 2 · Pointer traversal                       read memory, write nothing
    ├── ft_strlen → ft_strchr → ft_strrchr → ft_memchr
    └── ft_memset → ft_bzero
        You are learning: ...

A `Level N ·` line opens a level; each following `├──`/`└──` line is one
chain of `ft_*` names joined by `→`, optionally trailing a free-text caption
after two or more spaces (ignored here — it is prose, not data). A
`You are learning:` line closes the level.

Prerequisite rule
------------------
Two kinds of dependency live in that text, and both collapse into one flat
`prereqs` list per function — a function is "ready" once every name in its
own list has been written:

1. **Within a chain**, a function's prerequisite is whatever comes right
   before it in its `→` chain. `ft_strchr`'s prerequisite is `ft_strlen`;
   `ft_strrchr`'s is `ft_strchr`. Chains in the same level are independent of
   each other — the `ft_toupper` chain does not gate the `ft_isalpha` chain.

2. **Across levels**, the spec never spells out a numeric dependency, only
   the grouping into levels that each teach one more idea. Reading that as
   "every function in level N needs every function in level N-1" would let
   one slow function (say `ft_strtrim`) block an entire level for no
   textual reason, and the spec never asks for that. The rule used here is
   deliberately weaker: **the first function of a level's first chain (its
   "anchor" — what a student meets first on entering that level) becomes an
   extra prerequisite of every chain HEAD (the first function of each chain,
   which otherwise has no prerequisite) in the NEXT level.** One function
   written in level N-1 is enough to unlock the entry points of level N —
   "the previous level has been started", not "finished". Non-head
   functions do not carry the anchor directly; they already require their
   own chain's head, which carries it, so it is satisfied transitively by
   the time it would matter.

Level 1 has no level before it, so its chain heads (`ft_isalpha`,
`ft_toupper`) start with an empty prerequisite list.
"""

import argparse
import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT))

from bro42 import packs  # noqa: E402

BLOCK_RE = re.compile(r"```text\n(Level 1 ·.*?)\n```", re.S)
LEVEL_RE = re.compile(r"^Level (\d+) ·")
CHAIN_RE = re.compile(r"^[├└]──\s")
# Suite ids are lowercase identifiers - true of both Libft's `ft_name` and
# ft_printf's `pf_name`. The `ft_` requirement this used to carry was a Libft
# fact that did not belong in a generator every pack shares.
FN_RE = re.compile(r"[a-z][a-z0-9_]*")


class SpecError(Exception):
    pass


def parse_track(text):
    """Returns {level: [[fn, fn, ...], ...]} — chains, in source order."""
    m = BLOCK_RE.search(text)
    if not m:
        raise SpecError("no fenced progressive-track block found in plan/rank00/libft-02-learning.md")
    levels = {}
    level = None
    for line in m.group(1).splitlines():
        lm = LEVEL_RE.match(line)
        if lm:
            level = int(lm.group(1))
            levels.setdefault(level, [])
            continue
        if CHAIN_RE.match(line):
            if level is None:
                raise SpecError(f"chain line before any 'Level N ·' heading: {line!r}")
            # A caption may trail the chain after two or more spaces (see the
            # module docstring) - free text, and now that FN_RE is not
            # anchored to `ft_` it would otherwise be mistaken for names.
            chain_part = re.split(r"\s{2,}", line, maxsplit=1)[0]
            fns = FN_RE.findall(chain_part)
            if not fns:
                raise SpecError(f"chain line has no suite names: {line!r}")
            levels[level].append(fns)
    return levels


def build(levels):
    """Turns {level: [chains]} into {fn: {level, prereqs, order}}."""
    functions = {}
    prev_anchor = None
    order = 0
    for level in sorted(levels):
        chains = levels[level]
        anchor = chains[0][0] if chains and chains[0] else None
        for chain in chains:
            for i, fn in enumerate(chain):
                order += 1
                if i == 0:
                    prereqs = [prev_anchor] if prev_anchor else []
                else:
                    prereqs = [chain[i - 1]]
                functions[fn] = {"level": level, "prereqs": prereqs, "order": order}
        prev_anchor = anchor
    return functions


def validate(levels, functions, pack, spec, cases_path):
    errors = []
    if sorted(levels) != list(range(1, len(levels) + 1)):
        errors.append(f"levels are not 1..N contiguous: {sorted(levels)}")

    seen = {}
    for level, chains in levels.items():
        for chain in chains:
            for fn in chain:
                seen.setdefault(fn, []).append(level)
    dupes = {fn: lv for fn, lv in seen.items() if len(lv) > 1}
    if dupes:
        errors.append(f"function(s) appear in more than one place in the track: {dupes}")

    for fn, meta in functions.items():
        for p in meta["prereqs"]:
            if p not in functions:
                errors.append(f"{fn}: prerequisite '{p}' is not itself a tracked function")

    # A magic number before packs existed - now a fact about the PROJECT
    # (pack.expected_suite_count), not about this generator. Exists to catch
    # a spec that silently lost a function.
    if len(functions) != pack.expected_suite_count:
        errors.append(f"expected {pack.expected_suite_count} functions in the "
                      f"track, found {len(functions)}")

    # Cross-check against cases.json's own Level annotation, so the two specs
    # cannot silently disagree about where a function sits.
    if cases_path.is_file():
        cases_meta = json.loads(cases_path.read_text()).get("functions", {})
        for fn, meta in functions.items():
            other = cases_meta.get(fn, {}).get("level")
            if other is not None and other != meta["level"]:
                errors.append(
                    f"{fn}: {spec.name}'s track puts it in level {meta['level']}, "
                    f"cases.json's Level annotation says {other}")
    return errors



def spec_missing_ok(spec, out):
    """The authored spec is not shipped; the data generated from it is.

    A clone has <pack>/data/*.json committed and no _dev/ to regenerate them
    from, so `make` must not die here - the tool is perfectly usable. Only an
    absent spec AND absent output is a real error.
    """
    if spec.is_file():
        return False
    if out.is_file():
        print(f"  {spec.name} not present - keeping the committed "
              f"{out.name} (this is a clone, not the authoring tree)")
        return True
    raise SystemExit(
        f"neither {spec} nor {out} exists - nothing to generate from and "
        f"nothing to fall back on")


def main():
    p = argparse.ArgumentParser()
    p.add_argument("--pack", default="libft", help="which pack to generate for")
    p.add_argument("--check", action="store_true",
                   help="validate the spec, do not write the output")
    args = p.parse_args()

    pack = packs.get(args.pack)
    spec = ROOT / pack.spec_dir / f"{pack.spec_prefix}-02-learning.md"
    cases_path = pack.data_dir() / "cases.json"
    out = pack.data_dir() / "roadmap.json"

    if spec_missing_ok(spec, out):
        return 0
    try:
        levels = parse_track(spec.read_text())
    except SpecError as e:
        print(f"\n1 problem(s):\n  {e}", file=sys.stderr)
        return 1

    functions = build(levels)
    errors = validate(levels, functions, pack, spec, cases_path)

    print(f"levels {len(levels)}   functions {len(functions)}")
    if errors:
        print(f"\n{len(errors)} problem(s):", file=sys.stderr)
        for e in errors:
            print(f"  {e}", file=sys.stderr)
        return 1

    if not args.check:
        out.parent.mkdir(parents=True, exist_ok=True)
        payload = {"source": str(spec.relative_to(ROOT)),
                   "functions": functions}
        out.write_text(json.dumps(payload, indent=1, sort_keys=True) + "\n")
        print(f"wrote {out.relative_to(ROOT)}")
    else:
        print("track OK")
    return 0


if __name__ == "__main__":
    sys.exit(main())
