"""What one 42 project contributes, and how bro42 finds the right one.

A pack is DATA, not code. It names the artifact the project builds, the
suites it ships, the symbols those suites need, the axis labels the terminal
and dashboard print, and where its authored specs live. There are no hooks and
no subclasses: everything that differs between projects differs as a value in
one of these fields, so nothing outside this package has to learn that a
second project exists.

That restraint is deliberate. _dev/plan/platform/10-packs.md argues that a pack
abstraction designed from one project would fit exactly one project, and asks
for the second pack to be built by copying before anything is extracted. So
this manifest describes only what two real projects were observed to differ
on. The four things that doc says need more evidence - a second execution
model (run a binary, not a linked archive), a build matrix (-D BUFFER_SIZE=n),
verdicts that are not comparisons, and building two repositories together -
are deliberately absent. get_next_line and push_swap will need them; guessing
their shape now is how you get an abstraction that fits nothing.
"""

from pathlib import Path


class Pack:
    """One project's manifest. Every field is data; none of it is behaviour."""

    def __init__(self, *, id, display, rank, subject_version, artifact,
                 header, sources, tree, suites, parts, levels,
                 allowed_externals, macro_sections, readme_sections,
                 spec_dir, spec_prefix, markers, engine_dir,
                 expected_suite_count):
        self.id = id
        self.display = display
        self.rank = rank
        self.subject_version = subject_version
        # ("archive", "libft.a") today. ("binary", "push_swap") is what the
        # second execution model will add - the tuple shape is here so the
        # field does not have to change when it arrives, but nothing reads
        # anything but "archive" yet.
        self.artifact = artifact
        self.header = header
        # A closed set of basenames bro is willing to compile when the repo's
        # own Makefile produced no archive, or None for "do not guess".
        # This is defect #5 in build.py's docstring: the previous tester ran
        # `find -name '*.c'` and ar'd whatever it found, swept up the
        # student's own main.c, and got a duplicate-main link failure out of a
        # repository that compiles perfectly. A pack that cannot enumerate its
        # sources must not fall back to a glob - it reports the build failure.
        self.sources = sources
        self.tree = tree
        # suite id -> {"symbol": str, "part": int}. Two facts, not one:
        # presence is decided per SYMBOL by nm, skipping and the external
        # allowlist are decided per SUITE. For Libft the two coincide (a
        # suite IS a function); for ft_printf fifteen suites share one symbol.
        self.suites = suites
        self.parts = parts
        self.levels = levels
        self.allowed_externals = allowed_externals
        self.macro_sections = macro_sections
        self.readme_sections = readme_sections
        self.spec_dir = spec_dir
        self.spec_prefix = spec_prefix
        self.markers = markers
        self.engine_dir = engine_dir
        # gen_roadmap.py's validate() hardcoded "!= 43" before packs existed -
        # that count catches a spec that silently lost a function, and it is
        # a fact about the PROJECT, not about the generator, so it belongs
        # here rather than staying a magic number in shared code.
        self.expected_suite_count = expected_suite_count

    # -- derived views, so callers never rebuild these by hand ------------

    @property
    def suite_ids(self):
        return list(self.suites)

    @property
    def symbols(self):
        """Every distinct symbol the pack's suites call.

        Not the same as the suite list: ft_printf is fifteen suites over one
        symbol. Presence is decided per SYMBOL by nm; skipping is decided per
        SUITE. Conflating the two is what made this a manifest field.
        """
        return sorted({m["symbol"] for m in self.suites.values()})

    def symbol_of(self, suite):
        return self.suites[suite]["symbol"]

    def part_of(self, suite):
        """The part a suite belongs to, defaulting to the most permissive.

        macro.check_symbols indexes ALLOWED_EXTERNALS by this. Defaulting to
        the widest allowlist is deliberate: an unknown suite must not be
        reported as calling a forbidden function on the strength of a lookup
        miss. A false "you called write()" is worse than a missed one, because
        the student cannot disprove it.
        """
        meta = self.suites.get(suite)
        if not meta:
            return max(self.allowed_externals, key=lambda p:
                       len(self.allowed_externals[p]))
        return meta["part"]

    def missing_suites(self, defined):
        """Suite ids whose symbol is absent from the archive."""
        return [s for s, m in self.suites.items()
                if m["symbol"] not in defined]

    def data_dir(self):
        return Path(__file__).resolve().parent / self.id / "data"

    def __repr__(self):
        return f"<Pack {self.id}>"


def _load():
    from . import ft_printf, libft
    return {p.id: p for p in (libft.PACK, ft_printf.PACK)}


_PACKS = None


def all_packs():
    global _PACKS
    if _PACKS is None:
        _PACKS = _load()
    return _PACKS


def get(pack_id):
    packs = all_packs()
    if pack_id not in packs:
        known = ", ".join(sorted(packs))
        raise KeyError(f"unknown pack {pack_id!r} - bro42 knows: {known}")
    return packs[pack_id]


def detect(target):
    """Which project is this repository? Returns a Pack, or None.

    Markers are checked at the REPOSITORY ROOT only, and the first pack whose
    markers all match wins. Root-only matters: ft_printf and push_swap are
    both "Libft authorized", so a perfectly ordinary ft_printf repository
    contains a complete libft/ subdirectory. Looking at the root picks the
    outer project, which is the one the student is working on. Reaching the
    inner one is still possible and explicit:

        bro --pack libft --target ./libft
    """
    target = Path(target)
    for pack in _detection_order():
        if pack.markers(target):
            return pack
    return None


def _detection_order():
    """Most specific first.

    ft_printf is checked before libft because an ft_printf repository legally
    contains a libft, and libft's own marker is the weaker of the two.
    """
    packs = all_packs()
    return [packs["ft_printf"], packs["libft"]]
