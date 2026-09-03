"""Argument parsing and dispatch."""

import argparse
import json
import shutil
import sys
from pathlib import Path

from . import build, content, history, macro, micro, packs, render, report

# The union of every known pack's part keywords (`part1`, `part2`, ...) - used
# only to recognise a selector as a part-filter BEFORE the target, and
# therefore the pack, is known (see main()'s early selector classification).
# Filtering against a part number the CHOSEN pack turns out not to have is
# not an error; it simply matches nothing, same as any other empty selection.
PART_OF_SELECTOR = {f"part{n}": n for p in packs.all_packs().values()
                    for n in p.parts}


def parse_args(argv):
    p = argparse.ArgumentParser(prog="bro", add_help=True,
                                description="a 42 gym bro")
    p.add_argument("selector", nargs="?", default=None,
                   help="function or fn:id, e.g. ft_memmove or ft_memmove:03")
    p.add_argument("--target", default=None, help="path to the student repository")
    p.add_argument("--pack", default=None,
                   help="which project to test (default: auto-detect)")
    p.add_argument("-v", "--verbose", action="count", default=0)
    p.add_argument("-q", "--quiet", action="store_true")
    p.add_argument("--json", action="store_true", help="NDJSON to stdout, nothing else")
    p.add_argument("--no-web", action="store_true", help="(alpha: always on)")
    p.add_argument("--no-color", action="store_true")
    p.add_argument("--timeout", type=int, default=2000, metavar="MS")
    p.add_argument("--where", action="store_true", help="print the cache directory")
    p.add_argument("--clean", action="store_true",
                   help="remove this target's cache directory")
    p.add_argument("--concept", default=None, help="filter defense questions")
    p.add_argument("extra", nargs="*", help=argparse.SUPPRESS)
    p.add_argument("--no-macro", action="store_true", help="skip the build checks")
    p.add_argument("--no-history", action="store_true", help="do not record this run")
    p.add_argument("--version", action="store_true", help="print the version and exit")
    p.add_argument("--check-update", action="store_true",
                   help="ask the origin remote whether a newer version exists")
    p.add_argument("--serve", action="store_true",
                   help="dashboard on http://127.0.0.1:4242 with live re-run")
    # parse_args, not parse_known_args: an unrecognised flag must be an error.
    # parse_known_args silently swallowed --serve and --clean, so `bro --serve`
    # printed an ordinary report and looked like it had worked.
    return p.parse_args(argv)


def resolve_pack(args, target):
    """Explicit --pack wins; otherwise detect from the target's own markers.

    Detection failing is not a crash - it means bro42 does not recognise this
    repository as any project it knows, and the fix is either --pack or a
    different --target, so say exactly that.
    """
    if args.pack:
        try:
            return packs.get(args.pack), None
        except KeyError as e:
            return None, str(e)
    pack = packs.detect(target)
    if pack is None:
        known = ", ".join(sorted(packs.all_packs()))
        return None, (f"could not tell which project {target} is - bro42 "
                      f"knows: {known}\n  tell it which one:  bro --pack <id>")
    return pack, None


def resolve_suite(pack, fn):
    """`split`, `ft_split` and `pf_c`, `c` all address the same suite - an
    exact suite id, or the unique suite whose id ends in `_<fn>` (the
    convenience `split` -> `ft_split` used to get from a hardcoded `ft_`
    prefix; suite ids are arbitrary per pack now, so membership decides it)."""
    if fn in pack.suites:
        return fn
    candidates = sorted(s for s in pack.suites if s.endswith(f"_{fn}"))
    if len(candidates) == 1:
        return candidates[0]
    if len(candidates) > 1:
        raise SystemExit(
            f"bro: '{fn}' is ambiguous for {pack.display} - matches "
            f"{', '.join(candidates)}")
    raise SystemExit(f"bro: '{fn}' is not a suite of {pack.display} - "
                     f"known suites: {', '.join(sorted(pack.suites))}")


def normalise_selector(pack, sel):
    """`split`, `ft_split`, `ft_split:06` all address the same suite."""
    if not sel:
        return None, 0
    if Path(sel).is_dir():
        return None, 0
    fn, _, cid = sel.partition(":")
    fn = resolve_suite(pack, fn)
    return fn, int(cid) if cid else 0


def st_early(args):
    return render.make_style(args.no_color)


def main(argv=None):
    argv = list(sys.argv[1:] if argv is None else argv)
    args = parse_args(argv)

    target = args.target
    # These selectors are all keywords, never a directory or a function name -
    # capture the ones that change what runs, then clear so the generic
    # directory/function resolution below leaves them alone.
    mode = None
    part_filter = None
    if args.selector in PART_OF_SELECTOR:
        part_filter = PART_OF_SELECTOR[args.selector]
        args.selector = None
    elif args.selector in ("micro", "macro"):
        mode = args.selector
        args.selector = None
    elif args.selector in ("explain", "defense", "next", "debug"):
        args.selector = None
    if target is None and args.selector and Path(args.selector).is_dir():
        target = args.selector
        args.selector = None
    if target is None:
        target = Path.cwd()

    from . import paths, update
    if args.version:
        print(f"bro42 {__import__('bro42').VERSION}")
        return 0
    if args.check_update:
        return update.check()
    if args.clean:
        return cmd_clean(target)

    # Everything past this point touches the target's cache or build, which
    # is now laid out per pack - so the pack has to be known before any of it.
    pack, error = resolve_pack(args, target)
    if pack is None:
        sys.stderr.write(f"bro: {error}\n")
        return 2

    if args.where:
        print(paths.cache_dir(target, pack))
        return 0
    if args.serve:
        from . import serve as serve_mod
        return serve_mod.main(target, pack)

    # `bro explain <concept>` and `bro defense [function]` read the spec and
    # never need a target to be built.
    if argv and argv[0] == "explain":
        slug = argv[1] if len(argv) > 1 else ""
        # If the target already has a report, show how the student is doing on
        # this idea across every function that tests it.
        recs = None
        binary, info = build.prepare(target, pack)
        if binary:
            recs, _ = micro.run(binary, pack, skip=info.get("missing"))
        print(render.concept_card(pack, slug, st_early(args), recs))
        return 0
    if argv and argv[0] == "defense":
        fn = argv[1] if len(argv) > 1 and not argv[1].startswith("-") else None
        print(render.defense_cards(pack, st_early(args), args.concept, fn))
        return 0

    # `bro macro` never needs the micro binary linked - the build checks run
    # against a fresh copy of the target regardless (macro.py, B17).
    if mode == "macro":
        st = render.make_style(args.no_color)
        checks = [c.as_dict() for c in macro.audit(target, pack)]
        if args.json:
            for c in checks:
                print(json.dumps({"kind": "macro", **c}, sort_keys=True))
            return 0
        print(f"\n  {st.bold('bro42')}  {st.dim(str(target))}  "
             f"{st.dim('· ' + pack.display)}\n")
        print(render.macro_line(checks, st, pack))
        print()
        print("\n".join(render.macro_detail(checks, st)))
        return min(125, sum(1 for c in checks if c["status"] == "FAIL"))

    binary, info = build.prepare(target, pack)
    st = render.make_style(args.no_color)
    if binary is None:
        sys.stderr.write(f"bro: cannot test {target}\n  {info.get('error','unknown')}\n")
        return 126

    if argv and argv[0] == "next":
        return cmd_next(target, pack, st, binary, info)
    if argv and argv[0] == "debug":
        return cmd_debug(target, pack, st, binary, argv[1:])

    # `bro micro`: the counterpart of `bro macro` - the build checks are
    # skipped, everything else about a full run is unchanged.
    if mode == "micro":
        args.no_macro = True

    fn, cid = normalise_selector(pack, args.selector)
    only = f"{fn}:{cid}" if fn and cid else (fn if fn else None)
    records, err = micro.run(binary, pack, only=only, timeout_ms=args.timeout,
                             skip=list(info.get('missing') or [])
                             + list(info.get('blocked') or {}))
    content.annotate_missing(pack, records, info)
    if err.strip():
        sys.stderr.write(err)

    if part_filter is not None:
        records = [r for r in records if r.get("part") == part_filter]

    # A part filter is a view over a full run, not a suite of its own - like
    # selecting one function, it does not get to redefine build status or
    # history for the whole target.
    restricted = bool(fn) or part_filter is not None

    checks = None
    if not args.no_macro and not restricted:
        checks = [c.as_dict() for c in macro.audit(target, pack)]

    hist_delta = None
    if not args.no_history and not restricted:
        entries = history.read(target, pack)
        entry = history.summarise(records, checks)
        previous = entries[-1] if entries else None
        hist_delta = history.delta(previous, entry, entries + [entry])
        history.append(target, pack, entry)

    if args.json:
        for c in (checks or []):
            print(json.dumps({"kind": "macro", **c}, sort_keys=True))
        for r in records:
            print(json.dumps(r, sort_keys=True))
        return min(125, sum(1 for r in records if r["status"] in render.SCORED_BAD))

    if not records:
        sys.stderr.write("bro: no cases ran - check the selector\n")
        return 2

    page = None
    if not restricted:
        page = report.write(records, target, pack, checks, hist_delta)

    verbose = args.verbose or (1 if fn else 0)
    if args.quiet:
        scored = [r for r in records if r["status"] not in render.UNSCORED]
        ok = sum(1 for r in scored if r["status"] == "OK")
        print(f"{ok}/{len(scored)}")
    else:
        print(render.report(records, target, st, pack, verbose=verbose,
                            restricted=restricted,
                            macro=checks, hist_delta=hist_delta))
        if page:
            print(f"  {st.dim('→ ' + str(page))}")
        print(render.version_line(st))
        if page and not args.no_web:
            import webbrowser
            webbrowser.open(page.as_uri())

    return min(125, sum(1 for r in records if r["status"] in render.SCORED_BAD))


def cmd_clean(target):
    """`bro --clean`: remove only this target's cache directory - never
    another target's, never the cache root itself (03_ORCHESTRATOR.md)."""
    from . import paths
    d = paths.cache_root() / paths.target_key(target)
    if d.exists():
        shutil.rmtree(d)
        print(f"removed {d}")
    else:
        print(f"nothing to remove at {d}")
    return 0


def cmd_next(target, pack, st, binary, info):
    """`bro next` (plan/rank00/libft-02-learning.md): the first unwritten function in the
    progressive track whose prerequisites are already written, with its
    concept cards."""
    present = info.get("present") or set()
    road = content.roadmap(pack)
    ordered = sorted(road.items(), key=lambda kv: kv[1]["order"])

    for fn, meta in ordered:
        if fn in present:
            continue
        prereqs = meta.get("prereqs", [])
        if any(p not in present for p in prereqs):
            continue
        satisfied = [p for p in prereqs if p in present]
        print("\n".join(render.next_block(fn, meta["level"], satisfied, st)))
        slugs = sorted({k for c in content.load(pack)["cases"].values()
                        if c["fn"] == fn for k in c["kw"]})
        for slug in slugs:
            print(render.concept_card(pack, slug, st))
        return 0

    unwritten = [fn for fn, _ in ordered if fn not in present]
    blocked = []
    for fn in unwritten:
        missing = [p for p in road[fn].get("prereqs", []) if p not in present]
        blocked += [p for p in missing if p not in blocked]
    records, _ = micro.run(binary, pack, skip=info.get("missing"))
    failing = [r for r in records if r["status"] in render.SCORED_BAD]
    print("\n".join(render.next_done(st, failing, blocked if unwritten else [])))
    return 0


def cmd_debug(target, pack, st, binary, argv_rest):
    """`bro debug <fn> <id>` (plan/platform/06-terminal-ux.md): the gdb and
    valgrind command lines that reproduce exactly this case."""
    if len(argv_rest) < 2:
        sys.stderr.write("bro: usage: bro debug <function> <id>\n")
        return 2
    fn, id_raw = argv_rest[0], argv_rest[1]
    fn = resolve_suite(pack, fn)
    try:
        cid = int(id_raw)
    except ValueError:
        sys.stderr.write(f"bro: not a case id: {id_raw}\n")
        return 2
    case = content.case(pack, fn, cid)
    if not case:
        sys.stderr.write(f"bro: no such case {fn}:{cid}\n")
        return 2
    desc = case.get("group") or case.get("desc") or ""
    print("\n".join(render.debug_block(fn, cid, desc, binary, st)))
    return 0
