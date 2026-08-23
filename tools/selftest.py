#!/usr/bin/env python3
"""The handoff gate (plan/platform/09-selftest.md, decision B15).

A tester's failure mode is silent: it reports 43/43 forever and the student
finds out at evaluation. So the only question worth asking is whether it
detects bugs, and the only honest way to ask it is to show it code that is
wrong on purpose.

Five checks:
  1. mutants   - each seeded bug is caught, on exactly its case ids, with
                 exactly the expected status. Caught for the wrong reason is
                 a failure: it means the tester found it by accident.
  2. baseline  - the reference fixture produces exactly its known result
  3. static    - invariants over the source and the spec that no test can see
  4. determinism - three runs agree, or a measurement is unstable
  5. valgrind  - the engine itself leaks nothing, which is the precondition
                 for trusting any number it reports about someone else
"""

import json
import re
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT))

from tools import mutants  # noqa: E402

BAD = {"KO", "SIGSEGV", "SIGBUS", "SIGABRT", "TIMEOUT", "LEAK"}

# The reference Libft is a real 42 submission and is not published with this
# repository. Everything here needs one; point it somewhere with
# FT_BRO_REFERENCE=/path/to/a/working/libft.
REFERENCE = mutants.REFERENCE

# The reference fixture is clean across all six tiers.
#
# It has not always been. ft_bro found two real defects in it on first contact
# and both were fixed while this was being built:
#   ft_memmove  the unrolled d < s branch copied in chunks of four and dropped
#               the n % 4 tail, so an overlapping 3-byte move copied nothing
#   ft_split    ft_free() was commented out and word_in() never checked a
#               malloc result, so a failed word allocation returned a
#               partly-built array and leaked
# Both are reseeded as mutants (memmove_no_tail, and the rollback sweep on
# every ft_split case), so neither can come back unnoticed.
#
# ft_strchr:8 and ft_strrchr:8 were the third and fourth, found when the T0
# batch landed: the active implementations compared `*sp == uc` with sp a
# signed char * and uc an unsigned char, so a byte above 127 (e.g. 0xC3, which
# promotes to -61) could never equal uc (which promotes to 195). Neither
# function could find any byte above 127. Both are now fixed by casting the
# dereferenced byte to unsigned char before the comparison.
#
# ft_calloc:8 was the fifth, found when the T2 batch landed: `size > 65535 / n`
# capped every allocation at 64 KB, so ft_calloc(100000, 1) returned NULL where
# it had to succeed. Fixed by replacing the literal with SIZE_MAX.
#
# ft_lstmap was the sixth, found when the T5 batch landed, and at the time was
# NOT fixed - only ft_calloc was in scope for that fix. ft_lstmap_bonus.c's
# failure branch was `tmp = ft_lstnew(f(l->content)); if (!tmp) {
# ft_lstclear(&newl, del); return (NULL); }`. When f() succeeded but the
# immediately following ft_lstnew() call's own malloc failed, tmp was NULL and
# the branch cleaned up every node already LINKED into newl - but the content
# f() just produced was never linked anywhere, so it was never freed.
# Confirmed by hand (a throwaway harness replicating the injected-allocator
# semantics, scratchpad, not committed): forcing the node's own malloc to fail
# after its content succeeded leaked exactly that one content block, on cases
# 1-4, 6 and 7 (case 5 has no source and cannot trigger it). This was a real
# defect, not a test artifact: bro_lstmap's fixture is built with
# bro_alloc()/bro_free() (unwrapped engine scratch, see test_ft_lstmap.c),
# never through the wrapped allocator, specifically to rule that out as the
# cause. Fixed on 2026-08-23 by holding f()'s result in a local `content` and
# calling del(content) in the failure branch before ft_lstclear runs - the
# same defect is reseeded as the mutant lstmap_orphans_content.
REFERENCE_KNOWN_BAD = {}

# The reference has no README.md, which is a real Subject V failure and not
# something ft_bro should pretend away. Held as a baseline so the macro mutants
# are judged on what they add.
BASELINE_MACRO = {"README.md", "first line", "Description section",
                  "Instructions section", "Resources section",
                  "AI usage disclosed"}


def run(target, extra=(), macro=False):
    """The macro audit costs ~10 make invocations, so it is off unless a check
    actually needs it. History is off everywhere: the self-test must not
    depend on, or pollute, the run log."""
    cmd = [str(ROOT / "bro"), "--json", "--target", str(target),
           "--no-history", *extra]
    if not macro:
        cmd.append("--no-macro")
    r = subprocess.run(cmd, capture_output=True, text=True, cwd=str(ROOT))
    out = [json.loads(l) for l in r.stdout.splitlines() if l.startswith("{")]
    if not out:
        raise SystemExit(f"no cases ran for {target}\n{r.stderr}")
    return out


def failures(records):
    return {f"{r['fn']}:{r['id']}": r["status"]
            for r in records
            if r.get("kind") != "macro" and r["status"] in BAD}


def macro_failures(records):
    return {r["name"] for r in records
            if r.get("kind") == "macro" and r["status"] == "FAIL"}


def check_baseline():
    got = failures(run(REFERENCE))
    if got != REFERENCE_KNOWN_BAD:
        extra = {k: v for k, v in got.items() if REFERENCE_KNOWN_BAD.get(k) != v}
        gone = {k: v for k, v in REFERENCE_KNOWN_BAD.items() if got.get(k) != v}
        return got, [f"reference baseline moved - new/changed: {extra}, "
                     f"no longer failing: {gone}"]
    print(f"  ok   baseline             {len(got)} known failure(s)")
    return got, []


def check_mutants_compile():
    """Every mutant must build with the project's OWN Makefile and flags.

    A mutant that fails -Werror produces no archive, so ft_bro falls back to
    compiling the sources leniently - and the mutant then exercises the
    fallback path rather than the defect it was written to seed. That looked
    like a pass and was not one.
    """
    problems = []
    for name, (path, *_rest) in mutants.build_all(quiet=True).items():
        r = subprocess.run(["make", "re"], cwd=path, capture_output=True, text=True)
        if not (path / "libft.a").is_file():
            problems.append(
                f"mutant {name} does not build with its own Makefile:\n"
                + (r.stdout + r.stderr).strip().splitlines()[-1][:120])
    if not problems:
        print("  ok   mutants build        all seeded with the project's own flags")
    return problems


def check_mutants(baseline):
    """Judged on the DELTA from the baseline: every mutant is a copy of the
    reference and inherits its defects, so absolute comparison would fold
    those into every expectation and hide them."""
    problems = []
    for name, (path, expected, macro_expect, fixes) in \
            mutants.build_all(quiet=True).items():
        records = run(path, macro=bool(macro_expect))
        got = failures(records)
        delta = {k: v for k, v in got.items() if baseline.get(k) != v}
        lost = {k for k in baseline if k not in got} - fixes
        macro_got = macro_failures(records) - BASELINE_MACRO
        if delta != expected:
            problems.append(
                f"mutant {name}: seeded bug should produce {expected}, got {delta}")
        elif lost:
            problems.append(f"mutant {name}: baseline failures vanished: {sorted(lost)}")
        elif macro_expect and not macro_expect <= macro_got:
            problems.append(
                f"mutant {name}: macro checks {sorted(macro_expect)} should have "
                f"failed, got {sorted(macro_got)}")
        else:
            shown = sorted(delta) or sorted(macro_expect)
            print(f"  ok   {name:22s} {shown}")
    return problems


def check_static():
    problems = []

    # Engine scratch must bypass the interposed allocator, or the engine's own
    # buffers would be counted as the student's and every case would leak.
    # list.c is the deliberate exception: its fixtures are memory the student
    # is expected to free, so they must go through the wrapped allocator.
    allowed = {"alloc.c", "list.c", "util.c"}
    for src in sorted((ROOT / "engine" / "src").glob("*.c")):
        if src.name in allowed:
            continue
        body = re.sub(r"/\*.*?\*/", "", src.read_text(), flags=re.S)
        for fn in ("malloc", "calloc", "realloc"):
            if re.search(rf"(?<![_\w]){fn}\s*\(", body):
                problems.append(f"{src.name} calls {fn} directly - use bro_alloc")
    if not problems:
        print("  ok   allocator discipline no bare malloc outside alloc.c/list.c")

    # The spec grammar, prose completeness and concept coverage.
    for gen in ("gen_cases", "gen_concepts"):
        r = subprocess.run([sys.executable, f"ft_bro/data/{gen}.py", "--check"],
                           capture_output=True, text=True, cwd=str(ROOT))
        if r.returncode:
            problems.append(f"{gen} check failed:\n" + r.stderr)
    if not any("gen_" in p for p in problems):
        print("  ok   spec                grammar, prose, concepts, defense bank")

    # The tier tables in the spec and the design doc must agree. Tier labels
    # are what the Stage D handoff instruction is built from, so a mismatch
    # points a labour model at the wrong exemplar - which is exactly how
    # ft_lstmap came to be filed as T3 in one file and T5 in another.
    problems += check_tier_agreement()

    # Every concept a case is tagged with must have a card to open.
    cases = json.loads((ROOT / "ft_bro" / "data" / "cases.json").read_text())
    cards = json.loads((ROOT / "ft_bro" / "data" / "concepts.json").read_text())
    tagged = {k for c in cases["cases"].values() for k in c["kw"]}
    unbacked = sorted(tagged - set(cards["concepts"]))
    if unbacked:
        problems.append(f"cases tagged with concepts that have no card: {unbacked}")
    else:
        print(f"  ok   concept index      {len(tagged)} tags all resolve to a card")

    # Every case the engine can emit must have prose to render.
    data = json.loads((ROOT / "ft_bro" / "data" / "cases.json").read_text())
    records = run(REFERENCE)
    cases_only = [r for r in records if r.get("kind") != "macro"]
    orphans = [f"{r['fn']}:{r['id']}" for r in cases_only
               if f"{r['fn']}:{r['id']}" not in data["cases"]]
    if orphans:
        problems.append(f"engine emits cases with no spec entry: {orphans}")
    else:
        print(f"  ok   join                {len(cases_only)} emitted cases all have prose")
    return problems


def check_dashboard():
    """A2: the page must be self-contained. It has to render on a cluster
    machine with no internet and be safe to hand to a peer, and the previous
    revision of plan/platform/07-dashboard.md promised a file:// mode that could not work."""
    target = REFERENCE
    subprocess.run([str(ROOT / "bro"), "--target", str(target), "--no-web",
                    "--no-history"], capture_output=True, cwd=str(ROOT))
    cache = subprocess.run([str(ROOT / "bro"), "--where", "--target", str(target)],
                           capture_output=True, text=True, cwd=str(ROOT)).stdout.strip()
    page = Path(cache) / "report.html"
    if not page.is_file():
        return ["no report.html was written"]
    html = page.read_text()
    problems = []
    external = re.findall(r'(?:src|href)="(?:https?:)?//[^"]+', html)
    if external:
        problems.append(f"report.html loads from the network: {external[:3]}")
    for token in ("__DATA__", "/*__STYLE__*/", "/*__APP__*/"):
        if token in html:
            problems.append(f"template placeholder {token} was not substituted")
    m = re.search(r'<script id="bro-data" type="application/json">(.*?)</script>',
                  html, re.S)
    if not m:
        return problems + ["no inlined data block"]
    data = json.loads(m.group(1))
    js = (ROOT / "web" / "app.js").read_text()
    for key in sorted(set(re.findall(r"DATA\.(\w+)", js))):
        if key not in data:
            problems.append(f"app.js reads DATA.{key}, which the report does not carry")
    if not problems:
        print(f"  ok   dashboard           self-contained, {len(html) // 1024} KB, "
              f"no network")

    # Self-contained and complete is not the same as "draws something". The
    # failure those checks cannot see is a JS exception on load leaving a blank
    # page - structurally perfect data, nothing rendered. Execute it.
    if not shutil.which("node"):
        print("  skip render              node not installed")
        return problems
    r = subprocess.run(["node", str(ROOT / "tools" / "render_check.js"), str(page)],
                       capture_output=True, text=True)
    if r.returncode:
        problems.append("dashboard render check failed:\n  "
                        + r.stderr.strip().replace("\n", "\n  "))
    else:
        print(r.stdout.rstrip())
    return problems


def check_tier_agreement():
    """cases.json (generated from plan/rank00/libft-01-cases.md) vs the hand-
    maintained tier table in plan/platform/04-testdesign.md.

    That table stays hand-written on purpose. rank00/libft-03-inventory.md also
    lists every tier, but it is GENERATED from cases.json - checking one against
    the other would compare cases.json with itself and pass no matter what.
    """
    doc = (ROOT / "_dev" / "plan" / "platform" / "04-testdesign.md").read_text()
    declared = {}
    for row in re.finditer(r"^\| \*\*T(\d)\*\* \|.*?\| (.+?) — \*\*\d+\*\* \|$",
                           doc, re.M):
        tier = int(row.group(1))
        for fn in re.findall(r"`(ft_\w+)`", row.group(2)):
            declared[fn] = tier
    if not declared:
        return ["could not parse the tier table out of plan/platform/04-testdesign.md"]
    funcs = json.loads((ROOT / "ft_bro" / "data" / "cases.json").read_text())["functions"]
    bad = [f"{fn}: spec says T{m['tier']}, 04-testdesign says T{declared[fn]}"
           for fn, m in sorted(funcs.items())
           if fn in declared and declared[fn] != m["tier"]]
    unlisted = sorted(set(funcs) - set(declared))
    if unlisted:
        bad.append(f"not in the tier table: {unlisted}")
    if bad:
        return bad
    print(f"  ok   tier agreement      {len(declared)} functions, spec and design agree")
    return []


def check_determinism():
    """An unstable measurement shows up as phantom fixed/broke lines in the
    delta header, which is the fastest way to make run history untrustworthy."""
    target = REFERENCE
    runs = []
    for _ in range(3):
        runs.append([
            {k: v for k, v in r.items() if k != "ms"}
            for r in run(target) if r.get("kind") != "macro"
        ])
    if runs[0] != runs[1] or runs[1] != runs[2]:
        return ["three runs of the same fixture disagree"]
    print(f"  ok   determinism         3 runs identical ({len(runs[0])} cases)")
    return []


def check_valgrind():
    """Cross-check the engine's own allocation accounting against valgrind.

    THIS CHECK USED TO LIE. It regexed stderr for "definitely lost: N bytes"
    and treated a missing match as zero, so when valgrind could not start at
    all - it needs 32-bit debuginfo this machine does not have - it printed
    "ok, the engine leaks nothing of its own" without ever running. A check
    that reports success when the tool never ran is the exact failure mode
    this project exists to prevent, and it sat here undetected from stage B
    until a subagent noticed the symptom.

    So: prove valgrind actually ran before believing anything it did not say.
    """
    if not shutil.which("valgrind"):
        print("  skip valgrind            not installed")
        return []
    target = REFERENCE
    subprocess.run([str(ROOT / "bro"), "--json", "--target", str(target),
                    "--no-macro", "--no-history"],
                   capture_output=True, cwd=str(ROOT))
    cache = subprocess.run(
        [str(ROOT / "bro"), "--where", "--target", str(target)],
        capture_output=True, text=True, cwd=str(ROOT)).stdout.strip()
    binary = Path(cache) / "bro_micro"
    r = subprocess.run(
        ["valgrind", "--leak-check=full",
         str(binary), "--no-fork", "--only", "ft_memmove"],
        capture_output=True, text=True)

    # Evidence valgrind actually instrumented the program. Without one of
    # these it never got off the ground and has told us nothing.
    if "HEAP SUMMARY" not in r.stderr and "ERROR SUMMARY" not in r.stderr:
        why = next((l.replace("valgrind:", "").strip()
                     for l in r.stderr.splitlines()
                     if "valgrind:" in l and l.replace("valgrind:", "").strip()),
                    "no diagnostic")
        print(f"  skip valgrind            could not run - {why[:64]}")
        return []

    m = re.search(r"definitely lost: ([\d,]+) bytes", r.stderr)
    if not m:
        return ["valgrind ran but printed no leak summary - cannot conclude "
                "anything about the engine's own accounting"]
    lost = int(m.group(1).replace(",", ""))
    if lost:
        return [f"the engine itself leaks {lost} bytes - its own numbers "
                f"cannot be trusted"]
    print("  ok   valgrind            the engine leaks nothing of its own")
    return []


def check_subject_19_2_layout():
    """The same library, laid out the way subject 19.2 asks for it.

    19.2 IV.4 puts the nine list functions in the MANDATORY part, so a current
    repo names them ft_lstnew.c and builds them from `all` with no `bonus`
    rule. ft_bro used to warn twice at exactly that layout - "bonus file(s)
    not named _bonus.c" and "no working bonus rule" - telling a student who
    followed their own subject that they had it wrong.

    Compares every non-OK macro row, not just failures: both of those were
    WARN, so a FAIL-only comparison would sit through the regression.
    """
    src = REFERENCE
    work = mutants.OUT / "subject_19_2"
    if work.is_dir():
        shutil.rmtree(work)
    shutil.copytree(src, work, ignore=shutil.ignore_patterns(".git"))
    for old in list(work.glob("*_bonus.c")) + list(work.glob("*_bonus.h")):
        old.rename(work / old.name.replace("_bonus", ""))
    mk = work / "Makefile"
    text = mk.read_text().replace("_bonus.c", ".c")
    text = text.replace("SRCS_O = $(SRCS:.c=.o)", "SRCS += $(BONUS)\nSRCS_O = $(SRCS:.c=.o)")
    text = re.sub(r"^bonus: .*\n(\t.*\n)+", "", text, flags=re.M)
    mk.write_text(text.replace("$(BONUS_O)", ""))

    records = run(work, macro=True)
    rows = {r["name"]: r["status"] for r in records
            if r.get("kind") == "macro" and r["status"] != "OK"}
    unexpected = {k: v for k, v in rows.items() if k not in BASELINE_MACRO}
    micro = failures(records)
    problems = []
    if unexpected:
        problems.append(f"19.2 layout raises rows a legacy layout does not: {unexpected}")
    if micro:
        problems.append(f"19.2 layout changes micro results: {micro}")
    if not problems:
        print("  ok   subject 19.2 layout mandatory Part 3, no bonus rule, nothing raised")
    return problems


def main():
    print("selftest")
    if not REFERENCE.is_dir():
        print(f"\n  no reference Libft at {REFERENCE}", file=sys.stderr)
        print("  The self-test seeds deliberate bugs into a known-good Libft;\n"
              "  that fixture is not shipped with this repository.\n"
              "  FT_BRO_REFERENCE=/path/to/a/working/libft make selftest",
              file=sys.stderr)
        return 1
    baseline, problems = check_baseline()
    problems += check_mutants_compile()
    problems += check_mutants(baseline)
    problems += check_static()
    problems += check_dashboard()
    problems += check_determinism()
    problems += check_subject_19_2_layout()
    problems += check_valgrind()
    if problems:
        print(f"\nFAILED ({len(problems)})", file=sys.stderr)
        for p in problems:
            print(f"  {p}", file=sys.stderr)
        return 1
    print("\nselftest: pass")
    return 0


if __name__ == "__main__":
    sys.exit(main())
