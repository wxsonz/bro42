"""Terminal output (plan/platform/06-terminal-ux.md).

Quiet by default: the shape of the project, then only what is wrong, in full.
Colour is decoration and never the only signal - every status carries a word.
"""

import os
import re
import shutil
import sys
import textwrap
from pathlib import Path

from . import content

SCORED_BAD = {"KO", "SIGSEGV", "SIGBUS", "SIGABRT", "TIMEOUT", "LEAK"}
UNSCORED = {"UB", "MISSING", "SKIP"}


def _display(pack, fn):
    """The suite's short name for the terminal - "atoi", not "ft_atoi" - from
    cases.json's `display` key (defaulting to the suite id itself when the
    spec gives none). Replaces the `.replace("ft_", "")` this used to be:
    Libft's spec supplies "atoi" for "ft_atoi" as authored content, so the
    stripping is a fact about that ONE pack's spec, not code every pack runs."""
    return content.function(pack, fn).get("display", fn)


class Style:
    def __init__(self, enabled):
        self.on = enabled

    def _w(self, code, text):
        return f"\033[{code}m{text}\033[0m" if self.on else text

    def dim(self, t):
        return self._w("2", t)

    def bold(self, t):
        return self._w("1", t)

    def red(self, t):
        return self._w("31", t)

    def green(self, t):
        return self._w("32", t)

    def yellow(self, t):
        return self._w("33", t)

    def cyan(self, t):
        return self._w("36", t)


def make_style(no_color=False):
    enabled = (
        not no_color
        and sys.stdout.isatty()
        and os.environ.get("NO_COLOR") is None
        and os.environ.get("TERM") != "dumb"
    )
    return Style(enabled)


def width():
    return min(shutil.get_terminal_size((100, 24)).columns, 100)


def _char(byte):
    """Render one byte the way the spec writes it."""
    mapping = {0: "\\0", 10: "\\n", 13: "\\r", 9: "\\t"}
    if byte in mapping:
        return mapping[byte]
    if 32 <= byte < 127:
        return chr(byte)
    return f"{byte:02x}"


def byte_grid(rec, st, indent="        "):
    """Three aligned rows and a marker under the first divergence."""
    exp = rec.get("expected_bytes") or []
    act = rec.get("actual_bytes") or []
    if not exp:
        return []
    base = rec.get("window_base", 0)
    diverge = rec.get("diverge", -1)
    shown = min(len(exp), max(8, (diverge - base + 4) if diverge >= 0 else 8))
    shown = min(shown, len(exp))
    cells = [max(3, len(_char(exp[i])), len(_char(act[i]))) for i in range(shown)]

    def row(label, values, colour=None):
        out = [f"{label:>4} "]
        for i in range(shown):
            text = _char(values[i]).rjust(cells[i])
            if colour and diverge >= 0 and base + i == diverge:
                text = colour(text)
            out.append(text + " ")
        return indent + "".join(out)

    lines = [
        indent + "idx  " + "".join(str(base + i).rjust(cells[i]) + " " for i in range(shown)),
        row("exp", exp, st.green),
        row("got", act, st.red),
    ]
    if diverge >= 0 and diverge - base < shown:
        pad = sum(cells[i] + 1 for i in range(diverge - base))
        lines.append(indent + "     " + " " * pad + st.red("^" * cells[diverge - base]))
        lines.append(indent + "     " + " " * pad + st.dim(f"diverges at index {diverge}"))
    return lines


def rollback_line(rec, st, indent="        "):
    """One mark per allocation index, so a partial rollback localises itself:
    "you handle the first two failures and not the third" beats "you leak"."""
    roll = rec.get("rollback")
    if not roll:
        return []
    alloc = rec.get("alloc", {})
    # 1-based to match the "malloc #N forced to fail" message above.
    marks = " ".join(
        (st.green(f"✓#{k + 1}") if ok else st.red(f"✗#{k + 1}"))
        for k, ok in enumerate(roll))
    head = (f"{indent}{alloc.get('under_test', len(roll))} allocation(s); "
            f"forcing each to fail in turn:")
    return [head, f"{indent}  {marks}", ""]


def wrap(text, indent, first_label):
    body = textwrap.wrap(text, width=width() - len(indent) - 6) or [""]
    out = [f"{indent}{first_label:<5}{body[0]}"]
    out += [f"{indent}{'':<5}{line}" for line in body[1:]]
    return out


def failure_block(rec, st, pack):
    ind = "  "
    inner = "        "
    head = f"{ind}{st.bold(rec['fn'])} {rec['id']:02d}"
    label = rec.get("input") or ""
    status = rec["status"]
    tag = st.red(status) if status in SCORED_BAD else st.yellow(status)
    lines = [f"{head}   {tag}   {st.dim(label)}"]
    if rec.get("msg"):
        lines.append(f"{inner}{rec['msg']}")
    lines += byte_grid(rec, st)
    lines += rollback_line(rec, st)
    if rec.get("kind") in ("scalar", "offset") and "expected" in rec:
        lines.append(
            f"{inner}expected {st.green(str(rec['expected']))}"
            f"    got {st.red(str(rec['actual']))}"
        )
    lines.append("")
    if rec.get("why"):
        lines += wrap(rec["why"], inner, "why")
    # A failure produced by the allocation sweep is not about what the case was
    # written to check, so the case's own `why` does not explain it. Name the
    # concept that does.
    if rec.get("rollback") and rec.get("alloc", {}).get("fail_after", -1) >= 0:
        card = content.concept(pack, "malloc-rollback")
        if card:
            lines += wrap(card.get("title", ""), inner, "note")
    if status == "UB" and rec.get("note"):
        lines += wrap(rec["note"], inner, "note")
    elif rec.get("fix"):
        lines += wrap(rec["fix"], inner, "fix")
    kw_text = " · ".join(rec.get("kw", []))
    ref_text = ("ref: " + rec["ref"]) if rec.get("ref") else ""
    if kw_text or ref_text:
        used = len(inner) + 5 + len(kw_text) + len(ref_text)
        pad = max(2, width() - used)
        lines.append(f"{inner}{'kw':<5}{st.cyan(kw_text)}"
                     f"{' ' * pad}{st.dim(ref_text)}")
    lines.append("")
    return lines


_QUOTED = re.compile(r"'[^']*'")


def _error_signature(msg):
    """Same diagnosis, different file: 'unknown type name 'size_t'' in
    ft_memset.c and ft_memchr.c is one bug, not two - generalise away the
    one thing that legitimately differs per call site (the quoted name)."""
    return _QUOTED.sub("'…'", msg)


def shared_compile_errors(records):
    """Broken functions grouped by root cause, most-shared first. A single
    header mistake (a missing #include, an absent t_list) fails every
    function that happens to need it, and without this a student sees N
    unrelated-looking 'does not compile' names instead of the one line of
    libft.h to go fix."""
    groups = {}
    for r in records:
        if r.get("missing_reason") != "does not compile":
            continue
        err = r.get("compile_error")
        if not err:
            continue
        sig = _error_signature(err)
        group = groups.setdefault(sig, {"example": err, "fns": set()})
        group["fns"].add(r["fn"])
    shared = [(g["example"], sorted(g["fns"])) for g in groups.values()
             if len(g["fns"]) >= 2]
    shared.sort(key=lambda pair: -len(pair[1]))
    return shared


def bar(passed, total, filled=7):
    if not total:
        return "░" * filled
    n = round(filled * passed / total)
    return "█" * n + "░" * (filled - n)


def summary(records, st, pack):
    """One bar per LEARNING LEVEL, not per function - a level is the unit a
    student works through (the pack's own progressive track)."""
    by_level = {}
    for r in records:
        level = content.function(pack, r["fn"]).get("level", 0)
        by_level.setdefault(level, []).append(r)
    lines = []
    for level in sorted(by_level):
        rs = by_level[level]
        scored = [r for r in rs if r["status"] not in UNSCORED]
        ok = sum(1 for r in scored if r["status"] == "OK")
        broken = sorted({_display(pack, r["fn"]) for r in rs
                         if r["status"] in SCORED_BAD}
                        | {_display(pack, r["fn"]) for r in rs
                           if r.get("missing_reason") == "does not compile"})
        tag = f"L{level} {pack.levels.get(level, ''):<9}"
        if not scored:
            broke = sorted({_display(pack, r["fn"]) for r in rs
                            if r.get("missing_reason") == "does not compile"})
            if broke:
                lines.append(f"  {tag} {st.dim(bar(0, 0))}  "
                             + st.red("does not compile: " + " ".join(broke)))
            else:
                lines.append(f"  {tag} {st.dim(bar(0, 0))}  {st.dim('not written')}")
            continue
        mark = st.red("  ← " + " ".join(broken)) if broken else ""
        colour = st.green if not broken else st.red
        lines.append(f"  {tag} {colour(bar(ok, len(scored)))}  "
                     f"{ok}/{len(scored)}{mark}")
    return lines


MACRO_MARK = {"OK": "✓", "FAIL": "✗", "WARN": "!", "SKIP": "—"}


def macro_line(checks, st, pack):
    """One line in the default view: the shape of the build half."""
    by_section = {}
    for c in checks:
        by_section.setdefault(c["section"], []).append(c)
    names = pack.macro_sections
    parts = []
    for sec in sorted(by_section):
        cs = by_section[sec]
        # An unknown section number must not KeyError - it should just show
        # as itself rather than crash a report over a lookup miss.
        label = names.get(sec, str(sec))
        if any(c["status"] == "FAIL" for c in cs):
            parts.append(st.red(f"{label} ✗"))
        elif all(c["status"] == "SKIP" for c in cs):
            parts.append(st.dim(f"{label} —"))
        elif any(c["status"] == "WARN" for c in cs):
            parts.append(st.yellow(f"{label} !"))
        else:
            parts.append(st.green(f"{label} ✓"))
    return "  " + "   ".join(parts)


def macro_detail(checks, st):
    out = []
    shown = [c for c in checks if c["status"] != "OK"]
    if not shown:
        return out
    fails = [c for c in shown if c["status"] == "FAIL"]
    title = f" {len(fails)} build issue{'s' if len(fails) != 1 else ''} " \
        if fails else " build notes "
    out.append("── " + st.bold(title.strip()) + " "
               + "─" * max(0, width() - len(title) - 6))
    out.append("")
    for c in shown:
        mark = MACRO_MARK.get(c["status"], "?")
        colour = {"FAIL": st.red, "WARN": st.yellow}.get(c["status"], st.dim)
        out.append(f"  {colour(mark)} {st.bold(c['name'])}   {c['desc']}")
        if c.get("hint"):
            for line in c["hint"].splitlines():
                out += wrap(line, "      ", "")
        if c.get("command"):
            out.append(f"      {st.dim('ran: ' + c['command'])}")
        # For a build failure the compiler's own words are the most useful
        # thing on the screen, and they were only ever reaching the dashboard.
        if c["status"] == "FAIL" and c.get("output"):
            said = [l for l in c["output"].splitlines()
                    if "error:" in l or "Error" in l or "warning:" in l]
            for line in (said or c["output"].splitlines())[:8]:
                text = line.rstrip()
                room = width() - 10
                if len(text) > room:          # wrap, do not guillotine
                    text = text[:room - 1] + "…"
                out.append(f"      {st.red('│')} {text}")
        out.append("")
    return out


def delta_header(d, st, pack):
    """What changed since last time - the only thing a returning user wants
    first. `broke` is called out separately because a regression nobody
    noticed is what this tool exists to catch."""
    if not d:
        return []
    out = [f"  {st.dim('since ' + d['since'].replace('T', ' ')[:16])}"]
    for cid in d["fixed"]:
        out.append(f"    {st.green('✓ fixed')}    {cid.replace(':', ' ')}")
    for cid in d["broke"]:
        out.append(f"    {st.red('✗ broke')}    {cid.replace(':', ' ')}"
                   f"{st.red('   ← new since last run')}")
    if d["new_funcs"]:
        out.append(f"    {st.cyan('+ wrote')}    "
                   + " · ".join(_display(pack, f) for f in d["new_funcs"]))
    if d.get("gone_funcs"):
        out.append(f"    {st.yellow('- gone')}     "
                   + " · ".join(_display(pack, f) for f in d["gone_funcs"]))
    if d["from"] != d["to"]:
        was = f"{d['from'][0]}/{d['from'][1]}"
        now = f"{d['to'][0]}/{d['to'][1]}"
        out.append(f"    {st.dim(was)} → {st.bold(now)}")
    if d["streak"] > 1:
        out.append(f"    {st.yellow('streak')}     {d['streak']} days")
    if len(out) == 1:
        return []
    out.append("")
    return out


def unbuilt_notice(records, st, pack):
    """Warn when the pack declares suites this build of bro42 cannot run.

    Only meaningful on a whole-pack run. `records` is what actually ran, so
    on a filtered run (`bro ft_split:3`, `bro part2`) every suite the filter
    excluded looks absent, and the notice would tell a student their tool is
    broken when it is only doing what they asked. The caller passes
    restricted=True for those - see cli.main.
    """
    unbuilt = pack.unbuilt_suites({r.get("fn") for r in records})
    if not unbuilt:
        return []
    total = len(pack.suites)
    return [
        "  " + st.yellow(f"incomplete pack · {len(unbuilt)} of {total} "
                       f"{pack.display} suites are not built into this bro42"),
        "  " + st.dim("  " + " ".join(sorted(unbuilt))),
        "  " + st.dim("  the score below covers only the suites that ran - "
                      "it is not a verdict on your project"),
        "",
    ]


def report(records, target, st, pack, verbose=0, macro=None, hist_delta=None,
        restricted=False):
    out = []
    out.append("")
    out.append(f"  {st.bold('bro42')}  {st.dim(str(target))}  "
               f"{st.dim('· ' + pack.display)}")
    out.append("")
    if not restricted:
        out += unbuilt_notice(records, st, pack)
    out += delta_header(hist_delta, st, pack)
    out += summary(records, st, pack)
    out.append("")
    if macro:
        out.append(macro_line(macro, st, pack))
        out.append("")

    if macro:
        out += macro_detail(macro, st)
    failures = [r for r in records if r["status"] in SCORED_BAD]
    unscored = [r for r in records if r["status"] in UNSCORED]

    if failures:
        title = f" {len(failures)} failure{'s' if len(failures) != 1 else ''} "
        out.append("── " + st.bold(title.strip()) + " " + "─" * max(0, width() - len(title) - 6))
        out.append("")
        for rec in failures:
            out += failure_block(rec, st, pack)

    if verbose:
        out.append("── " + st.dim("all cases") + " " + "─" * max(0, width() - 16))
        for rec in records:
            mark = st.green("✓") if rec["status"] == "OK" else (
                st.yellow("·") if rec["status"] in UNSCORED else st.red("✗"))
            kw = st.dim(" · ".join(rec.get("kw", [])))
            out.append(f"  {mark} {rec['fn']} {rec['id']:02d}  {rec.get('input',''):<44} {kw}")
        out.append("")

    scored = [r for r in records if r["status"] not in UNSCORED]
    ok = sum(1 for r in scored if r["status"] == "OK")
    # Cases for the score, FUNCTIONS for what is missing - a student wants to
    # know how many functions are left, not how many cases those would be.
    extra = [f"{v} {k}" for k, v in sorted(
        {k: v for k, v in
         ((s, sum(1 for r in unscored if r["status"] == s)) for s in ("UB",))
         if v}.items())]
    tail = f"  {ok}/{len(scored)} scored"
    if extra:
        tail += st.dim("  ·  " + " · ".join(extra))
    out.append("─" * width())
    out.append(tail)
    fn_state = {}
    for r in records:
        fn = r["fn"]
        state = r.get("missing_reason") if r["status"] == "MISSING" else "written"
        if state:
            fn_state[fn] = state
    written = sum(1 for v in fn_state.values() if v == "written")
    broke = sorted(f for f, v in fn_state.items() if v == "does not compile")
    blocked = sorted(f for f, v in fn_state.items() if v == "blocked")
    absent = sum(1 for v in fn_state.values() if v == "not written")
    if broke or absent or blocked:
        parts = [f"{written} written"]
        if broke:
            parts.append(st.red(f"{len(broke)} does not compile"))
        if blocked:
            parts.append(st.yellow(f"{len(blocked)} blocked"))
        if absent:
            parts.append(st.dim(f"{absent} not written"))
        out.append("  " + " · ".join(parts) + f"  of {len(fn_state)}")
        if broke:
            out.append(st.red("  fix first:  " + " ".join(broke)))
            for msg, fns in shared_compile_errors(records):
                names = " ".join(f.replace("ft_", "") for f in fns)
                out.append(st.dim(f"    {len(fns)}x same error: {msg}  ({names})"))
        if blocked:
            # name the dependency, not just the blocked function - the whole
            # point is that the thing to go write is the one NOT listed here
            need = sorted({d for r in records
                           for d in r.get("blocked_on", [])})
            out.append(st.yellow("  blocked:    " + " ".join(blocked))
                       + st.dim("  waiting on " + " ".join(need)))
    out.append("")
    return "\n".join(out)


def plain(text, st):
    """The cards are authored as markdown in the spec; the terminal wants
    prose. Fences go, `code` becomes cyan, *emphasis* loses its asterisks.

    Code spans are lifted out BEFORE emphasis is stripped and put back after.
    Stripping first ate the pointer stars inside them - `void *` rendered as
    `void `, and `memchr(const void *, int c, size_t)` lost its asterisk - and
    an unpaired backtick left over from a span that wrapped across lines then
    swallowed the rest of the paragraph.
    """
    if text.strip() in ("```", "```text", "```c"):
        return None
    spans = []

    def stash(m):
        spans.append(m.group(1))
        return f"\x00{len(spans) - 1}\x00"

    text = re.sub(r"`([^`]+)`", stash, text)
    text = re.sub(r"\*\*([^*]+)\*\*", r"\1", text)
    text = re.sub(r"(?<!\w)\*([^*]+)\*(?!\w)", r"\1", text)
    # a lone backtick means the span wrapped a line; show it as plain text
    text = text.replace("`", "")
    return re.sub(r"\x00(\d+)\x00", lambda m: st.cyan(spans[int(m.group(1))]),
                  text)


def concept_card(pack, slug, st, records=None):
    """`bro explain <slug>`. When results are available the card carries a live
    roll-up - which is the thing a static card cannot do: it tells a student
    that the idea failing in one function is the idea passing in another."""
    card = content.concept(pack, slug)
    if not card:
        known = ", ".join(sorted(content.concepts(pack)))
        return f"bro: no concept '{slug}'\n\nknown concepts:\n  {known}"
    out = ["", f"  {st.bold('CONCEPT')} · {st.cyan(slug)}", ""]
    out += wrap(plain(card["title"], st) or "", "  ", "")
    out.append("")
    for line in card["body"].splitlines():
        rendered = plain(line, st)
        if rendered is not None:
            out.append("  " + rendered)
    out.append("")
    if card["functions"]:
        out.append(f"  {st.dim('tested in')}  " + " ".join(card["functions"]))
    if card["refs"]:
        out.append(f"  {st.dim('reference')}  " + " · ".join(card["refs"]))
    if records is not None:
        tagged = [r for r in records if slug in r.get("kw", [])]
        if tagged:
            ok = sum(1 for r in tagged if r["status"] == "OK")
            colour = st.green if ok == len(tagged) else st.red
            out.append("")
            out.append(f"  {st.dim('in your ' + pack.display.lower())}  "
                       f"{colour(f'{ok}/{len(tagged)}')} of the cases that test this pass")
            for r in tagged:
                if r["status"] != "OK":
                    out.append(f"      {st.red('✗')} {r['fn']} {r['id']:02d}  "
                               f"{r.get('input', '')}")
    for q in card["defense"]:
        out.append("")
        out.append(f"  {st.dim('defense')}    \"{plain(q, st)}\"")
    out.append("")
    return "\n".join(out)


def next_block(fn, level, satisfied, st):
    """`bro next` (plan/rank00/libft-02-learning.md): the chosen function and why it is ready.
    The concept cards themselves are appended by the caller via concept_card -
    this renderer only draws the "here is what's next, and why" part."""
    out = ["", f"  {st.bold('next')} → {st.cyan(fn)}   {st.dim('level ' + str(level))}"]
    if satisfied:
        out.append(f"  {st.dim('prerequisites written:')} " + ", ".join(satisfied))
    else:
        out.append(f"  {st.dim('no prerequisites - this starts the track')}")
    out.append("")
    return out


def next_done(st, failing, blocked):
    """Everything ready has already been written. Either the whole track is
    done (say so, and point at anything still failing instead), or nothing
    unwritten has its prerequisites satisfied yet (say what is blocking)."""
    out = [""]
    if not blocked:
        out.append(f"  {st.green('every function in the progressive track is written.')}")
        if failing:
            out.append("")
            out.append(f"  {st.dim('still not passing:')}")
            for r in failing:
                out.append(f"    {st.red('✗')} {r['fn']} {r['id']:02d}  "
                           f"{r.get('input', '')}")
        else:
            out.append(f"  {st.dim('and every scored case passes.')}")
    else:
        out.append(f"  {st.yellow('nothing unwritten is ready yet')} "
                   f"- waiting on: " + ", ".join(blocked[:6]))
    out.append("")
    return out


def _pretty_path(p):
    """~/.cache/... reads better than the absolute form and is just as
    pasteable - every shell that runs gdb also expands a leading ~."""
    home = str(Path.home())
    s = str(p)
    return "~" + s[len(home):] if s.startswith(home + "/") else s


def debug_block(fn, cid, desc, binary, st):
    """`bro debug <fn> <id>` (plan/platform/06-terminal-ux.md) - printed verbatim
    there, down to the column where the valgrind continuation line lines up
    under its own command name. Both command lines must be pasteable as-is,
    so no colour goes inside them even when colour is on."""
    label = f"{fn}:{cid}"
    bin_str = _pretty_path(binary)
    gdb_cmd = f"gdb --args {bin_str} --no-fork --only {label}"
    val_head = "valgrind --leak-check=full --show-leak-kinds=all \\"
    val_tail = f"{bin_str} --no-fork --only {label}"
    head = f"  {st.bold(label)}   {desc}".rstrip()
    out = ["", head, ""]
    out.append(f"  {'gdb':<8}  {gdb_cmd}")
    out.append(f"  {'valgrind':<8}  {val_head}")
    out.append(" " * 21 + val_tail)
    out.append("")
    out.append("  --no-fork disables crash isolation so gdb catches the fault "
               "in its own process.")
    out.append("")
    return out


def defense_cards(pack, st, concept_filter=None, function_filter=None):
    bank = content.defense_bank(pack)
    if concept_filter:
        bank = [q for q in bank if concept_filter in q["concepts"]]
    if function_filter:
        bank = [q for q in bank if function_filter in q["question"]
                or function_filter in q["answer"]]
    if not bank:
        return "bro: no defense questions match that filter"
    out = [""]
    for q in bank:
        out.append(f"  {st.bold('Q' + str(q['n']))}  {plain(q['question'], st)}")
        out.append(f"      {st.cyan(' · '.join(q['concepts']))}")
        out += wrap(plain(q["answer"], st) or "", "      ", "")
        out.append("")
    return "\n".join(out)


def version_line(st):
    """The closing line of every run: which bro this is, and whether a newer
    release is known to exist.

    `update.pending()` only ever reads a local file left by --check-update
    (A18): no network call happens here, so this costs nothing and works
    unchanged on a machine with no internet.
    """
    from . import VERSION, paths, update

    latest = update.pending()
    if not latest:
        return f"  {st.dim('bro42 ' + VERSION)}\n"
    return (f"  {st.dim('bro42 ' + VERSION)}   "
            f"{st.yellow(latest + ' is available')} "
            f"{st.dim('· git -C ' + str(paths.ROOT) + ' pull && make')}\n")
