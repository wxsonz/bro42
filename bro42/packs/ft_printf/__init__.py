"""ft_printf (subject v12.1) - rank01. The second pack.

Shape notes that are NOT true of Libft, and are the reason several fields in
the Pack manifest exist at all:

  - One symbol, fifteen suites. `ft_printf` is a single function, but testing
    it as one 300-case blob is unreadable in the terminal and useless in the
    dashboard's function grid. The suites are conversions, so `suites` maps
    fifteen ids onto one symbol.

  - No basename allowlist. The subject's turn-in list is
    `Makefile, *.h, */*.h, *.c, */*.c` - subdirectories are allowed and the
    filenames are the student's choice. There is no closed set to compile, so
    `sources` is None and a missing archive is reported rather than guessed
    at. See the Pack.sources comment for why guessing is forbidden.

  - "Libft authorized: Yes", so the repository legally contains a whole
    libft/ subtree with its own Makefile, which the project's Makefile calls.
    `tree` therefore copies one directory level down, or `make` fails in the
    working copy.

  - Chapter V asks for a sixth README section this project's Libft does not:
    "A detailed explanation and justification of the chosen algorithm and data
    structure must also be included."
"""

from .. import Pack

SYMBOL = "ft_printf"

# Suite ids are lowercase only, and %X is `pf_xup` rather than `pf_X`, because
# `pf_x` and `pf_X` differ only by case - and so would the C files named after
# them. On a case-insensitive filesystem, which is every 42 Mac, those two
# files are one file. An uglier id is cheaper than that failure.
MANDATORY_SUITES = [
    "pf_literal", "pf_pct", "pf_c", "pf_s", "pf_d", "pf_i",
    "pf_u", "pf_x", "pf_xup", "pf_p", "pf_ret", "pf_robust",
]

BONUS_SUITES = ["pf_width", "pf_precision", "pf_flags"]

# Subject IV, "External functs." The va_* names are compiler builtins on
# x86-64 and never appear as undefined symbols, so listing them changes
# nothing today - but the subject grants them and the allowlist should say
# what the subject says, not what one architecture happens to emit.
_EXTERNALS = {"malloc", "free", "write",
              "va_start", "va_arg", "va_copy", "va_end"}


def _markers(target):
    """Root markers only - an ft_printf repo legally contains a whole libft."""
    if (target / "ft_printf.h").is_file():
        return True
    makefile = target / "Makefile"
    if makefile.is_file():
        try:
            return "libftprintf.a" in makefile.read_text(errors="replace")
        except OSError:
            return False
    return False


PACK = Pack(
    id="ft_printf",
    display="ft_printf",
    rank="r01",
    subject_version="12.1",
    artifact=("archive", "libftprintf.a"),
    header="ft_printf.h",
    sources=None,
    tree=("Makefile", "makefile", "*.c", "*.h", "*.md",
          "*/Makefile", "*/*.c", "*/*.h"),
    suites={
        **{s: {"symbol": SYMBOL, "part": 1} for s in MANDATORY_SUITES},
        **{s: {"symbol": SYMBOL, "part": 2} for s in BONUS_SUITES},
    },
    parts={1: "mandatory", 2: "bonus"},
    levels={
        1: "literal text", 2: "variadic mechanics", 3: "signed decimal",
        4: "unsigned & bases", 5: "pointers", 6: "the return count",
        7: "robustness", 8: "flags, width, precision",
    },
    allowed_externals={1: set(_EXTERNALS), 2: set(_EXTERNALS)},
    macro_sections={
        1: "structure", 2: "rules", 3: "relink", 4: "flags",
        5: "symbols", 6: "archive", 7: "norm", 8: "README", 9: "pitfalls",
    },
    readme_sections=("Description", "Instructions", "Resources", "algorithm"),
    spec_dir="_dev/plan/rank01",
    spec_prefix="ftprintf",
    markers=_markers,
    engine_dir="engine/packs/ft_printf",
    expected_suite_count=15,
)
