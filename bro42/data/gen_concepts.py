#!/usr/bin/env python3
"""Generate bro42/data/concepts.json from _dev/plan/rank00/libft-02-learning.md.

Same principle as gen_cases.py (decision A14): the teaching content is authored
once, in the spec, and the tool reads it. A concept card that lived in both
places would drift, and the drift would be invisible.

Grammar:

    ## 1 · `signed-char`

    **A one-line definition in bold.**

    ...body, which may contain fenced blocks...

    **Tested in:** `ft_strncmp` `ft_memcmp` ...
    **References:** `man 3 memcmp` · C99 6.2.5
    **Defense:** *"Why did you cast to unsigned char * in ft_memcmp?"*

Plus the defense bank:

    ### Q3 · Why does ft_strlcat return ... · `return-contract`
    > model answer, possibly several lines
"""

import argparse
import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT))

from bro42 import packs  # noqa: E402

CARD_RE = re.compile(r"^## \d+ · `([a-z-]+)`\s*$")
FIELD_RE = re.compile(r"^\*\*(Tested in|References|Defense):\*\*\s*(.*)$")
Q_RE = re.compile(r"^### Q(\d+) · (.+?)\s*·\s*(.+)$")


def split_ticks(text):
    return re.findall(r"`([^`]+)`", text)


def split_refs(text):
    parts = [p.strip(" `*") for p in text.split("·")]
    return [p for p in parts if p]


def parse_cards(lines):
    cards = {}
    slug = None
    body = []
    for line in lines:
        m = CARD_RE.match(line)
        if m:
            if slug:
                cards[slug]["body"] = "\n".join(body).strip()
            slug = m.group(1)
            cards[slug] = {"slug": slug, "title": "", "body": "",
                           "functions": [], "refs": [], "defense": []}
            body = []
            continue
        if not slug:
            continue
        if line.startswith("---"):
            cards[slug]["body"] = "\n".join(body).strip()
            slug = None
            continue
        m = FIELD_RE.match(line)
        if m:
            key, val = m.group(1), m.group(2)
            if key == "Tested in":
                cards[slug]["functions"] = split_ticks(val)
            elif key == "References":
                cards[slug]["refs"] = split_refs(val)
            else:
                # The field is *"the question?"* and may be followed by a
                # remark ("- the canonical question"). Take the quoted part.
                m2 = re.search(r'\*"(.+?)"\*', val)
                cards[slug]["defense"].append(
                    m2.group(1) if m2 else val.strip(' *"'))
            continue
        if not cards[slug]["title"] and line.startswith("**") and line.endswith("**"):
            cards[slug]["title"] = line.strip("*").strip()
            continue
        body.append(line)
    if slug:
        cards[slug]["body"] = "\n".join(body).strip()
    return cards


def parse_defense(lines):
    bank = []
    current = None
    for line in lines:
        m = Q_RE.match(line)
        if m:
            if current:
                bank.append(current)
            current = {
                "n": int(m.group(1)),
                "question": m.group(2).strip(),
                "concepts": split_ticks(m.group(3)),
                "answer": [],
            }
            continue
        if current is not None:
            if line.startswith("> "):
                current["answer"].append(line[2:].strip())
            elif line.startswith("#") or line.startswith("---"):
                bank.append(current)
                current = None
    if current:
        bank.append(current)
    for q in bank:
        q["answer"] = " ".join(q["answer"]).strip()
    return bank



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
    out = pack.data_dir() / "concepts.json"

    if spec_missing_ok(spec, out):
        return 0
    lines = spec.read_text().splitlines()
    cards = parse_cards(lines)
    defense = parse_defense(lines)

    problems = []
    for slug, card in cards.items():
        if not card["title"]:
            problems.append(f"concept {slug}: no one-line definition")
        if not card["functions"]:
            problems.append(f"concept {slug}: no 'Tested in' list")
        if not card["defense"]:
            problems.append(f"concept {slug}: no defense question")
    for q in defense:
        if not q["answer"]:
            problems.append(f"defense Q{q['n']}: no model answer")
        for c in q["concepts"]:
            if c not in cards:
                problems.append(f"defense Q{q['n']}: unknown concept '{c}'")

    print(f"concepts {len(cards)}   defense questions {len(defense)}")
    if problems:
        print(f"\n{len(problems)} problem(s):", file=sys.stderr)
        for p in problems:
            print(f"  {p}", file=sys.stderr)
        return 1
    if not args.check:
        out.parent.mkdir(parents=True, exist_ok=True)
        out.write_text(json.dumps(
            {"source": str(spec.relative_to(ROOT)), "concepts": cards,
             "defense": defense}, indent=1, sort_keys=True) + "\n")
        print(f"wrote {out.relative_to(ROOT)}")
    else:
        print("concepts OK")
    return 0


if __name__ == "__main__":
    sys.exit(main())
