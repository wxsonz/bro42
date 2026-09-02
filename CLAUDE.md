# CLAUDE.md

Guidance for Claude Code working in this repository.

## What this is

`bro42` is a teaching companion and test suite for the 42 **Libft** project (subject v19.2).
It is deliberately **not a grader**: every case carries prose explaining what it was for, shown
on passes as well as failures. Run as `bro` from inside a student's libft repository.

Three surfaces: **micro** (299 authored cases across 43 functions), **macro** (build/Makefile/
Norm/README checks, each citing a subject clause), and **study** (concept cards, defense
questions, a searchable HTML dashboard).

## Architecture

C engine + Python orchestrator + static web page (decision A1).

```
bro                  bash launcher; resolves symlinks, execs `python3 -m bro42`. Never cd's -
                     the target defaults to $PWD.
engine/              the C micro engine. Calls student functions, emits NDJSON facts, nothing
                     else. Built into build/{core,packs/<pack>}/*.o by the Makefile; linked per
                     target by build.py.
  include/bro.h      core's public header: t_status, t_ctx, t_case, t_suite, flags. Frozen at
                     stage B - all six tiers T0-T5 use t_ctx. Project-agnostic - no pack (e.g.
                     Libft's t_list) may put its types here.
  core/              harness (fork per case), guard (canaries), alloc (--wrap injection),
                     capture (fd), oracle, emit (NDJSON), assert, ctx, util, main. Knows nothing
                     project-specific.
  packs/libft/       the Libft pack: pack.h (t_list and friends), proto.h (the subject's
                     prototypes), registry.c (static suite list), list.c (t_list fixtures,
                     ownership tracking), tests/partN/ (one file per function, cases
                     transcribed from the spec).
bro42/              the Python orchestrator.
  cli.py             arg parsing and dispatch
  build.py           compile the target, nm audit, link bro_micro against their libft.a
  micro.py/macro.py  run the two suites
  content.py         join engine facts to spec prose by fn:id
  render.py          terminal output; report.py  report.json + self-contained report.html
  history.py         append-only JSONL, since-last-run delta
  serve.py           127.0.0.1:4242, stdlib only, exists only to power Re-run buttons
  update.py          opt-in `--check-update`; compares against the newest release tag
  data/*.json        GENERATED from _dev specs and committed. bro refuses to start without them.
  data/gen_*.py      the generators
web/                 template.html + app.js + style.css, inlined into report.html
tools/               selftest.py, mutants.py, gen_inventory.py, render_check.js
_dev/                GITIGNORED. Authored specs, design ledger, reference libft, PDFs.
```

Data flow: engine emits `{"fn","id","status",...}` and **no prose** — `why`/`fix`/`kw`/`ref`
come from `bro42/packs/<pack>/data/cases.json` at render time, so rewording an explanation rebuilds nothing.

## Commands

```sh
make                 # build engine objects + regenerate data JSONs
make cases           # regenerate data JSONs only (needs _dev/)
make check           # verify committed JSONs match the specs (needs _dev/)
make selftest        # the handoff gate (needs _dev/ or BRO42_REFERENCE_LIBFT)
make re / fclean / clean

BRO42_REFERENCE_LIBFT=/path/to/a/working/libft make selftest
node tools/render_check.js <report.html>    # headless dashboard smoke test
python3 tools/gen_inventory.py              # regenerate the _dev inventory view
```

Runtime: `bro`, `bro ft_split`, `bro ft_split:13`, `bro macro`, `bro explain <concept>`,
`bro defense`, `bro --json`, `bro --serve`, `bro --where`, `bro --check-update`.

## Hard constraints

**Nothing is written into the student's repository** (A9). `bro` runs their `make` — those `.o`
and `libft.a` are their own build — and everything else goes to `~/.cache/bro42/<sha of repo
path>/`. `git status` must stay clean; the macro suite's own "no unused files" check would
otherwise flag bro's output.

**No network on its own** (A2/A8). `bro --check-update` is the only thing that touches it. No
pip packages, no npm, no CDN — stdlib Python, plain C, hand-written JS. The dashboard loads
nothing remote.

**Case IDs are permanent addresses** (B7). Append only; never renumber a case, in the spec or in
the engine. Decision numbers in `_dev/plan/platform/01-decisions.md` are append-only too — a
resolved `C` entry keeps a pointer from its old number.

**`fix` names the mechanism, never hands over code** (A11). The generator enforces this.

**Linux-first, degrade on macOS** (A8). Allocation injection and leak accounting need GNU ld's
`--wrap`; on macOS those cases report `SKIP` and everything else works. Never let a missing
capability become a failure — same for a missing `norminette`.

**Statuses `UB`, `MISSING` and `SKIP` never score** (A5). `UB` cases still run and still display,
showing the student's behaviour beside glibc's, explicitly labelled.

## Working on this

- **Specs are the source of case prose.** `_dev/plan/rank00/libft-01-cases.md` is authored;
  `cases.json` is generated from it. Changing an explanation means editing the spec and running
  `make cases` — never hand-editing the JSON. A clone without `_dev/` cannot do this, which is
  why the JSONs are committed.
- **Adding a case:** write it in the spec, `make cases`, then transcribe it into the matching
  `engine/packs/libft/tests/partN/test_ft_*.c` with the same id. The engine's `g_cases[]` id and
  the spec id must agree, or the prose joins to the wrong case.
- **Adding a function:** a line in `engine/packs/libft/registry.c`, a line in the Makefile's
  `LIBFT_TESTS`, an entry in `bro42/packs/libft/__init__.py`. No dynamic registration.
- **Part is a permission boundary, not a difficulty ordering** (`bro42/packs/libft/__init__.py`): subject IV.3
  lets Part 2 call `write`; Parts 1 and 3 may not. The symbol audit depends on this.
- Engine C is ordinary C99 with 42-ish conventions (tabs, `t_` typedefs, `g_` globals) but is
  **not** norminette-constrained — one-line `static void case_01(...)` definitions are normal here.
- Comments in this codebase explain *why*, usually citing a spec section or a defect that
  motivated the choice. Match that when editing; do not strip those citations.
- **`make selftest` is the gate.** It seeds mutants into a known-good libft and asserts each is
  caught on exactly the expected case ids with the expected status — caught for the wrong reason
  is a failure. A tester's failure mode is silent, so changes to the engine or the case data
  should be validated here, not by eyeballing a report.
