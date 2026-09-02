# bro42

A pair-programming, guidance and testing companion for 42 **Libft** (subject v19.3).

```
cd ~/my_libft && bro
```

---

## Description

**bro42 is not a grader.** It tells you what each test was *for* — including the ones you pass,
because that's what you repeat at defense.

- **Tests your Libft** — 299 authored cases across the 43 functions, each with a rationale, a fix
  that names the mechanism without handing you code, concept tags and a reference. Every case runs
  in its own forked process, so one segfault doesn't cost you the whole report.
- **Checks your build** — Makefile rules, relinking, compiler flags, forbidden symbols, archive
  integrity, Norm and README, each citing the subject clause it comes from.
- **Helps you study** — sixteen concepts that cut across functions, peer-evaluation questions with
  model answers, and a searchable dashboard.

It writes nothing into your repository — reports, run history and the test binary live in
`~/.cache/bro42/`, so `git status` stays clean.

It also catches what a return-value check can't: leaks and failed-allocation rollback (via
`malloc` interception at the linker), out-of-bounds writes (via poison canaries), and crashes,
hangs and unwritten functions, each classified separately from a wrong answer.

---

## Instructions

### Install

```sh
git clone <this repository> bro42
cd bro42
./install.sh            # explains each step, asks before doing it
./install.sh --dry-run  # show what it would do, change nothing
```

Requires `cc`, `make`, `ar`, `nm` and `python3`. No pip packages, no Node, no network.
`norminette` is optional — without it the Norm check reports `SKIP` rather than failing you for a
tool you don't have.

Linux is the primary platform. On macOS the allocation-failure and leak checks report `SKIP`
(they need GNU `ld`'s `--wrap`); everything else works.

### Use

```sh
bro                      # everything, then open the dashboard
bro ft_split             # one function, verbose
bro ft_split:13          # one case
bro macro                # build checks only

bro explain overlap      # a concept card, with how you are doing on it
bro defense              # peer-evaluation questions and model answers
bro --json               # machine-readable; exit code is the failure count
bro --where              # where the cache lives
bro --version
bro --check-update       # opt-in; bro never touches the network on its own
```

`-v` expands passing cases, `-q` prints the score line only, `--no-web` skips the browser.

`bro` never contacts the network on its own. `bro --check-update` compares your version against
the newest release tag and remembers what it found, so later runs remind you until you update.

---

## Resources

The 42 Libft subject (v19.3), `man 3` pages for the libc functions being reimplemented (BSD man
pages for `strlcpy`/`strlcat`/`strnstr`), ISO C99, and `ld(1)` on `--wrap` for the allocation
interception.

**How AI was used**: this tool was designed and written with Claude — specs, C engine, Python
orchestrator, web dashboard and self-test. The work was directed, reviewed and corrected by a
human throughout.

**What is not in this repository**: the authored specifications the case prose is written in, and
the reference Libft used as the self-test fixture — both are one person's 42 submission. `bro`
runs fine from a clone (`bro42/packs/<pack>/data/*.json` is generated from those specs and committed here),
but a clone can't regenerate that data (`make cases`) or run the self-test (`make selftest`)
without pointing it at a working Libft:

```sh
BRO42_REFERENCE_LIBFT=/path/to/a/working/libft make selftest
```

It seeds deliberate bugs into a copy of that Libft and checks each is caught — it never modifies
the libft you point it at.

---

## Is bro42 itself tested?

Yes — a tester's failure mode is silent. A broken web app looks broken; a broken tester reports
43/43 forever and you find out at evaluation.

```sh
make selftest
```

Seeds deliberate bugs into a known-good Libft and asserts each is caught on exactly the expected
case IDs with the expected status — a mutant caught for the wrong reason is a failure. Plus
allocator discipline, spec grammar, concept coverage, determinism across runs, and a valgrind
check that the engine itself leaks nothing.
