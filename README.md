# bro42

A pair-programming, guidance and testing companion for 42 **Libft** (subject v19.2).

```
cd ~/my_libft && bro
```

---

## Description

**bro42 is not a grader.** A grader tells you a number and speaks only when you are wrong.
`bro` tells you what each test was *for* — including the ones you pass, because that sentence is
what you repeat at defense.

Concretely, it does three things:

- **Tests your Libft** — 299 authored cases across the 43 functions, each carrying its own
  rationale, a fix that names the mechanism without handing you code, concept tags and a
  reference. Every case runs in its own forked process, so a segfault in one function costs you
  that case and not the whole report.
- **Checks your build** — Makefile rules, relinking, compiler flags, forbidden symbols, archive
  integrity, Norm and README, each citing the subject clause it comes from and showing the exact
  command it ran.
- **Helps you study** — sixteen concepts that cut *across* functions, so you find out that the
  thing failing in `ft_memcmp` is the thing passing in `ft_strncmp`; plus peer-evaluation
  questions with model answers, and a searchable dashboard.

It writes nothing into your repository. Reports, run history and the test binary all live in
`~/.cache/bro42/`, so `git status` stays clean.

### What it can find that a return-value check cannot

- **Leaks and failed-allocation rollback.** `malloc` is intercepted at the linker, so `bro` can
  force the *k*-th allocation of `ft_split` to fail and watch whether you free the words you
  already built. It reports which *k* you stop coping at.
- **Writes past the end of a buffer.** Every destination carries poison canaries, so an
  `ft_memset` that writes `n + 1` bytes is caught even though its return value and visible
  output are both correct.
- **Crashes, hangs and unwritten functions**, each classified separately from a wrong answer.

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
`norminette` is optional — without it the Norm check reports `SKIP` with an install hint rather
than failing you for a tool you do not have.

Linux is the primary platform. On macOS the allocation-failure and leak checks turn off (they
need GNU `ld`'s `--wrap`, which Apple's linker has no equivalent for) and those cases report
`SKIP`; everything else works.

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

### Versions and updates

Every run ends with the version it used, and the dashboard header carries it too — a report you
keep is a report you can place in time.

`bro` never contacts the network on its own. `bro --check-update` is the only thing that does,
and it compares your version against the newest release tag; ordinary commits do not count, so
you are told when there is a release, not when a typo was fixed. It remembers what it found, and
until you update, later runs remind you:

```
  bro42 1.0.0   1.1.0 is available · git -C /path/to/bro pull && make
```

Run it whenever you like — nothing expires, and `bro` works exactly the same with no internet at
all.

---

## Resources

**Reference material**

- The 42 Libft subject, version 19.2 — the source of every `fail` severity in the build checks.
- `man 3` pages for the libc functions being reimplemented; the BSD man pages for `strlcpy`,
  `strlcat` and `strnstr`.
- ISO C99 (§6.2.5 character types, §6.3.1.3 unsigned conversion, §6.5.6 pointer arithmetic,
  §3.4.3 undefined behaviour) for the cases where "it works on my machine" is not an argument.
- `ld(1)` on `--wrap`, for the allocation interception.

**How AI was used**

This tool was designed and written with Claude, and that covers essentially all of it: the
specifications it is generated from, the C engine, the Python orchestrator, the web dashboard
and the self-test. The work was directed, reviewed and corrected by a human throughout — several designs
were rejected outright, and the AI's own mistakes were caught by the self-test it had written.

It is disclosed here for the same reason `bro` checks for a disclosure in *your* README: Chapter
V asks for one, and a tool that makes that check should not be quiet about its own answer.

**On bro42's own README**

Running `bro`'s Chapter V validator against this file gives 4 of 5: Description, Instructions,
Resources and the AI disclosure all pass. The first-line check fails, and deliberately so — it
requires the line *"This project has been created as part of the 42 curriculum by &lt;login&gt;"*,
which would be false here. bro42 is a tool, not a curriculum submission. Chapter V governs your
Libft, not this; the four checks that do apply are met, and faking the fifth to score a clean
report would be exactly the habit this tool exists to argue against.

**What is not in this repository**

Two things are deliberately absent, because they are one person's machine and one person's 42
submission:

- **the authored specifications** the case prose is written in, and the design ledger recording
  every decision;
- **the reference Libft** used as the self-test fixture, and the mutants generated from it.

`bro` itself runs fine from a clone: the data it needs (`bro42/packs/<pack>/data/*.json` — case
rationales, concept cards, the progressive track) is generated from those specs and committed here.

What a clone cannot do is regenerate that data (`make cases`) or run the self-test
(`make selftest`), because both read sources that are not shipped. Point the self-test at any
working Libft to run it:

```sh
BRO42_REFERENCE_LIBFT=/path/to/a/working/libft make selftest
```

It will seed deliberate bugs into a copy of it and check that each is caught — it never modifies
the libft you point it at.

**On finding real bugs**

The Libft this was developed against was not a straw man; it was a real, passing 42 submission.
bro42 found **seven** genuine defects in it, every one verified independently of the tester
before being reported: `ft_memmove` dropping the `n % 4` tail of an unrolled overlapping copy,
`ft_split` returning a partly-built array without freeing, `ft_strchr` and `ft_strrchr` unable to
find any byte above 127, `ft_calloc` silently refusing every allocation of 64 KB or more,
`ft_lstmap` orphaning content when a node allocation failed, and `ft_memcmp` declaring a
parameter without the `const` the subject requires. All are fixed, and each is reseeded as a
mutant so it cannot return unnoticed.

Three of those were invisible until a **missing test case** was added. The suite kept finding its
own blind spots, which is the property worth having.

---

## Is bro42 itself tested?

Yes, and this is the part that matters most: **a tester's failure mode is silent.** A broken web
app looks broken. A broken tester reports 43/43 forever and you find out at evaluation.

```sh
make selftest
```

Seeds deliberate bugs into a known-good Libft — a forward-copying `memmove`, a signed `memcmp`, a
`lstclear` that frees content directly instead of through `del`, a Makefile with `.PHONY:
$(NAME)` — and asserts each is caught on **exactly** the expected case IDs with the **expected
status**. A mutant caught for the wrong reason is a failure: it means the tester found it by
accident. Plus allocator discipline, spec grammar, concept coverage, determinism across runs, and
a valgrind check that the engine leaks nothing of its own.
