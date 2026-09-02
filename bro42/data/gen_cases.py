#!/usr/bin/env python3
"""Generate <pack>/data/cases.json from that pack's authored case spec.

The spec is the single authored source for case prose (decision A14): why / fix /
kw / ref are written once, there, and this turns them into the sidecar the engine's
NDJSON output is joined against at render time.

Grammar (see "How to read this document" in plan/rank00/libft-01-cases.md - every
pack's spec follows the same grammar, only the suite ids and the spec location
differ, and both of those come from the pack manifest, not from this file):

    ### <n>. `suite_id` (display)
    - **Prototype:** ... **Part** 1 . **Level** 3 . **Tier** T0
    ```text
    group: <label>
    why:   <text, continued on indented lines>
    fix:   <text>
    kw:    <slug>, <slug>
    ref:   <man page or subject chapter>
    flags: ORACLE, GUARDED
      07  <call>  => <expectation>   [note]
    ```

The trailing `(display)` is optional - a suite whose spec omits it is displayed
under its own id. Libft's spec gives every function one, stripping its `ft_`
prefix, which is why the terminal and dashboard show "atoi" rather than
"ft_atoi" - that convention lives in the spec text, not in this generator.

Cases inherit the group's why/fix/kw/ref/flags and may override any of them with
their own indented key: line.
"""

import argparse
import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT))

from bro42 import packs  # noqa: E402

# Suite ids are lowercase identifiers, one or more `word_word` segments - true
# of both Libft's `ft_name` and ft_printf's `pf_name`. The `ft_` requirement
# this used to carry was a Libft fact that did not belong in a generator every
# pack shares.
# The display is deliberately looser than the suite id. An id has to be a
# safe identifier - it names a C file, a selector and a deep link - but a
# display is prose: ft_printf's suites are called %c, %X and %%, which no
# identifier class can spell. Bounded to one line and stopped at the closing
# paren, so a runaway display cannot swallow the rest of the heading.
FN_RE = re.compile(r"^### \d+\. `([a-z][a-z0-9_]*)`(?:\s+\(([^)\n]{1,20})\))?")
META_RE = re.compile(r"\*\*Part\*\*\s+(\d+).*?\*\*Level\*\*\s+(\d+).*?\*\*Tier\*\*\s+T(\d)")
ORACLE_RE = re.compile(r"^- \*\*Oracle:\*\*\s+(.+?)\s*(?:·|$)")
REF_RE = re.compile(r"\*\*Ref:\*\*\s+(.+?)\s*(?:·|$)")
GROUP_RE = re.compile(r"^group:\s*(.*)$")
FIELD_RE = re.compile(r"^(why|fix|note|kw|ref|flags):\s*(.*)$")
CONT_RE = re.compile(r"^ {4,}(\S.*)$")
CASE_RE = re.compile(r"^  (\d{2})  (\S.*?)  +=> (.*?)\s*$")
INDENT_FIELD_RE = re.compile(r"^ {6}(why|fix|note|kw|ref|flags):\s*(.*)$")

VALID_FLAGS = {"ORACLE", "UB", "INJECT", "GUARDED", "CAPTURES_FD"}

# A11: `fix` names the mechanism and never hands over a line to paste.
# A plain ";" test does not work — the prose uses semicolons ("count while *s is
# non-zero; do not add 1 at the end"). What distinguishes code is a brace, a real
# assignment, or a statement terminator.
CODE_PATTERNS = (
    re.compile(r"[{}]"),
    re.compile(r"\w\s*=\s*[\w'\"(&*]"),   # assignment, after comparisons are stripped
    re.compile(r";\s*$"),
    re.compile(r"\)\s*;"),
)
COMPARISON_RE = re.compile(r"[<>=!]=")


def looks_like_code(text):
    stripped = COMPARISON_RE.sub("", text)
    return any(p.search(stripped if i == 1 else text) for i, p in enumerate(CODE_PATTERNS))


class SpecError(Exception):
    pass


def concept_slugs(learn_path):
    text = learn_path.read_text()
    return set(re.findall(r"^## \d+ · `([a-z-]+)`", text, re.M))


def split_expectation(raw):
    """`> 0    mirror of 07` -> ('> 0', 'mirror of 07')."""
    parts = re.split(r"\s{3,}", raw, maxsplit=1)
    expect = parts[0].strip()
    note = parts[1].strip() if len(parts) > 1 else ""
    return expect, note


def parse(path):
    lines = path.read_text().splitlines()
    # The "How to read this document" section contains a worked example in the same
    # grammar. Parsing starts at the first Part heading so the example is not a case.
    for i, line in enumerate(lines):
        if line.startswith("# Part 1"):
            lines = lines[i:]
            break
    else:
        raise SpecError("no '# Part 1' heading found")
    cases, functions, errors = {}, {}, []
    fn = None
    group = {}
    last_key = None
    pending = None  # case dict still able to receive indented overrides

    def flush():
        nonlocal pending
        if pending is not None:
            cases[pending["id"]] = pending
            pending = None

    for lineno, line in enumerate(lines, 1):
        m = FN_RE.match(line)
        if m:
            flush()
            fn = m.group(1)
            functions[fn] = {"name": fn, "order": len(functions) + 1,
                             "display": m.group(2) or fn}
            group, last_key = {}, None
            continue

        if fn and line.startswith("- **Prototype:**"):
            m = META_RE.search(line)
            if m:
                functions[fn].update(
                    part=int(m.group(1)), level=int(m.group(2)), tier=int(m.group(3))
                )
            proto = re.search(r"`([^`]+)`", line)
            if proto:
                functions[fn]["prototype"] = proto.group(1)
            continue

        if fn and line.startswith("- **Oracle:**"):
            m = ORACLE_RE.match(line)
            if m:
                functions[fn]["oracle"] = m.group(1).strip("` ")
            m = REF_RE.search(line)
            if m:
                functions[fn]["ref"] = m.group(1).strip("` ")
            continue

        if fn and line.startswith("- **Ref:**"):
            m = REF_RE.search(line)
            if m:
                functions[fn]["ref"] = m.group(1).strip("` ")
            continue

        m = GROUP_RE.match(line)
        if m:
            flush()
            group = {"group": m.group(1).strip()}
            last_key = None
            continue

        m = INDENT_FIELD_RE.match(line)
        if m and pending is not None:
            key, val = m.group(1), m.group(2).strip()
            pending[key] = val
            last_key = ("case", key)
            continue

        m = FIELD_RE.match(line)
        if m and pending is None:
            key, val = m.group(1), m.group(2).strip()
            group[key] = val
            last_key = ("group", key)
            continue

        m = CONT_RE.match(line)
        if m and last_key:
            scope, key = last_key
            target = group if scope == "group" else pending
            if target is not None and key in target:
                target[key] = (target[key] + " " + m.group(1).strip()).strip()
            continue

        m = CASE_RE.match(line)
        if m:
            flush()
            if fn is None:
                errors.append(f"{path.name}:{lineno}: case line before any function heading")
                continue
            num = int(m.group(1))
            expect, note = split_expectation(m.group(3))
            cid = f"{fn}:{num}"
            if cid in cases:
                errors.append(f"{path.name}:{lineno}: duplicate id {cid}")
            pending = {
                "id": cid,
                "fn": fn,
                "num": num,
                "input": m.group(2).strip(),
                "expected": expect,
                "desc": note,
                "group": group.get("group", ""),
                "why": group.get("why", ""),
                "fix": group.get("fix", ""),
                "note": group.get("note", ""),
                "kw": group.get("kw", ""),
                "ref": group.get("ref", ""),
                "flags": group.get("flags", ""),
                "line": lineno,
            }
            last_key = ("case", None)
            continue

        # a case line that ALMOST matches is a grammar error, not something to skip
        if re.match(r"^  \d{2}  ", line):
            errors.append(
                f"{path.name}:{lineno}: case line has no '  => ' separator: {line.strip()[:60]}"
            )
            flush()

    flush()
    return functions, cases, errors


def normalise(cases, functions):
    for c in cases.values():
        if not c["ref"]:
            c["ref"] = functions.get(c["fn"], {}).get("ref", "")
        c["kw"] = [k.strip() for k in c["kw"].split(",") if k.strip()]
        c["flags"] = [f.strip() for f in c["flags"].split(",") if f.strip()]
    return cases


def validate(functions, cases, errors, spec, learn):
    slugs = concept_slugs(learn)
    seen_ids = {}
    for cid, c in cases.items():
        where = f"{spec.name}:{c['line']} {cid}"
        if not c["why"]:
            errors.append(f"{where}: no why — every case must say what it is for (A4)")
        if "UB" in c["flags"]:
            if not c["note"]:
                errors.append(f"{where}: UB case has no note")
        elif not c["fix"]:
            errors.append(f"{where}: no fix")
        if looks_like_code(c["fix"]):
            errors.append(f"{where}: fix looks like pasteable code (A11): {c['fix'][:60]}")
        if not c["kw"]:
            errors.append(f"{where}: no concept tags")
        if len(c["kw"]) > 4:
            errors.append(f"{where}: {len(c['kw'])} concept tags, max 4")
        for k in c["kw"]:
            if k not in slugs:
                errors.append(f"{where}: unknown concept '{k}'")
        for f in c["flags"]:
            if f not in VALID_FLAGS:
                errors.append(f"{where}: unknown flag '{f}'")
        seen_ids.setdefault(c["fn"], []).append(c["num"])

    for fn, nums in seen_ids.items():
        if nums != list(range(1, len(nums) + 1)):
            errors.append(f"{fn}: ids are not 1..n in order -> {nums}")

    used = {k for c in cases.values() for k in c["kw"]}
    for dead in sorted(slugs - used):
        errors.append(f"concept '{dead}' is carried by no case")

    for fn, meta in functions.items():
        if "tier" not in meta:
            errors.append(f"{fn}: no Part/Level/Tier line")
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
    spec = ROOT / pack.spec_dir / f"{pack.spec_prefix}-01-cases.md"
    learn = ROOT / pack.spec_dir / f"{pack.spec_prefix}-02-learning.md"
    out = pack.data_dir() / "cases.json"

    if spec_missing_ok(spec, out):
        return 0
    functions, cases, errors = parse(spec)
    cases = normalise(cases, functions)
    errors = validate(functions, cases, errors, spec, learn)

    slugs = concept_slugs(learn)
    print(f"functions {len(functions)}   cases {len(cases)}   concepts {len(slugs)}")

    if errors:
        print(f"\n{len(errors)} problem(s):", file=sys.stderr)
        for e in errors[:40]:
            print(f"  {e}", file=sys.stderr)
        if len(errors) > 40:
            print(f"  ... and {len(errors) - 40} more", file=sys.stderr)
        return 1

    if not args.check:
        out.parent.mkdir(parents=True, exist_ok=True)
        payload = {
            "source": str(spec.relative_to(ROOT)),
            "functions": functions,
            "cases": cases,
        }
        out.write_text(json.dumps(payload, indent=1, sort_keys=True) + "\n")
        print(f"wrote {out.relative_to(ROOT)}")
    else:
        print("grammar OK")
    return 0


if __name__ == "__main__":
    sys.exit(main())
