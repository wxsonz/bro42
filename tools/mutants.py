#!/usr/bin/env python3
"""Generate mutant libfts: one copy of the reference with one seeded bug.

plan/platform/09-selftest.md, decision B15. A tester's failure mode is silent - it
reports 43/43 forever - so the only way to know it works is to show it code
that is wrong and check it says so, for the RIGHT case and the right reason.

_dev/reference/libft_broke/ is advertised as broken code but is md5-identical
to libft42git/, so it proves nothing. Mutants are generated, never committed,
so they cannot drift from the reference.
"""

import os
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
# BRO42_REFERENCE_LIBFT is the current name (Phase 0 renamed the project from
# ft_bro to bro42); FT_BRO_REFERENCE is honoured too so an existing setup
# keeps working.
REFERENCE = Path(os.environ.get("BRO42_REFERENCE_LIBFT") or os.environ.get(
    "FT_BRO_REFERENCE") or ROOT / "_dev" / "reference" / "libft42git")


def require_reference():
    """The mutants need a known-good Libft to seed bugs into.

    That fixture is one person's 42 submission and is deliberately not
    published, so a clone has to be pointed at one.
    """
    if REFERENCE.is_dir():
        return
    raise SystemExit(
        f"no reference Libft at {REFERENCE}\n"
        "  The self-test seeds deliberate bugs into a known-good Libft, and\n"
        "  that fixture is not shipped with this repository.\n"
        "  Point it at one:  BRO42_REFERENCE_LIBFT=/path/to/a/working/libft make selftest")
OUT = ROOT / "_dev" / "fixtures" / "mutants"

# name -> (file, snippet, replacement, {case id: expected status})
#
# Expectations name the STATUS, not just the id: a mutant caught as SIGSEGV
# when it should be KO was found by accident, and the self-test has to be able
# to tell those apart.
#
# Replacement is applied to EVERY occurrence. A partial mutation is worse than
# no mutation: seeding only the first of ft_memcmp's two `unsigned char` casts
# left p1 signed and p2 unsigned, which made ft_memcmp:5 pass BY ACCIDENT
# (0 - 128 is negative, and negative was the expected sign). The expectations
# would then have been wrong in a way that looked correct.
MUTANTS = {
    # Always copy forward, even when dest > src. This is the bug memmove
    # exists in order not to have.
    "memmove_forward": (
        "ft_memmove.c",
        "if (d < s)",
        "if (1)",
        # Seeded at the DISPATCH, not by replacing the else branch: replacing
        # it leaves ft_copy_bwd unused, which -Werror rejects, and a mutant
        # that will not compile silently tests the build fallback instead of
        # the bug it was written for.
        {"ft_memmove:3": "KO", "ft_memmove:4": "KO"},
    ),
    # Drop the remainder loop from the unrolled forward copy, so any length
    # that is not a multiple of four loses its tail. This is the defect the
    # reference itself shipped until it was fixed on 2026-08-22.
    "memmove_no_tail": (
        "ft_memmove.c",
        "\t\tn -= 4;\n\t}\n\twhile (n-- > 0)\n\t\t*d++ = *s++;",
        "\t\tn -= 4;\n\t}",
        {"ft_memmove:5": "KO"},
    ),
    "memcmp_signed": (
        "ft_memcmp.c",
        "unsigned char",
        "char",
        {"ft_memcmp:4": "KO", "ft_memcmp:5": "KO", "ft_memcmp:6": "KO"},
    ),
    # ft_intlen widens to long precisely so -num cannot overflow on INT_MIN.
    # Narrowing it back is the classic itoa crash.
    "itoa_int_overflow": (
        "ft_itoa.c",
        "\tint\t\tlen;\n\tlong\tnum;",
        "\tint\t\tlen;\n\tint\t\tnum;",
        # SIGABRT, not KO, and that is the honest answer: with an int
        # accumulator -num overflows on INT_MIN, ft_intlen returns 2, itoa
        # mallocs 3 bytes and writes 11, and glibc's heap check aborts before
        # any wrong string can be compared. The bug is caught by the crash it
        # causes rather than by the value it returns.
        {"ft_itoa:12": "SIGABRT"},
    ),
    # A digit-count loop that never terminates: proves the alarm fires and is
    # classified as TIMEOUT rather than as a crash.
    "itoa_spin": (
        "ft_itoa.c",
        "while (num >= 10)",
        "while (num >= 0)",
        {f"ft_itoa:{i}": "TIMEOUT" for i in range(1, 14)},
    ),
    # Freeing the content directly instead of routing it through del. Only the
    # cases that count del calls can see this - which is why they exist.
    "lstclear_free_not_del": (
        "ft_lstclear.c",
        "del((*l)->content);",
        "free((*l)->content);",
        {"ft_lstclear:2": "KO", "ft_lstclear:3": "KO", "ft_lstclear:6": "KO"},
    ),
    # Free the array on rollback but not the words already in it. This is the
    # half-correct answer that leaks silently, and it reseeds the defect the
    # reference itself shipped until ft_free()/free_all() was wired up.
    "split_leaks_words": (
        "ft_split.c",
        "\t\ti--;\n\t\tfree(wordarr[i]);",
        "\t\ti--;\n\t\t(void)wordarr[i];",
        # Only the MULTI-word cases can see this: a single-word split has no
        # earlier word to leak when its own allocation fails. Cases 3, 6, 7 and
        # 12 are one-word inputs and are correctly silent here - which is the
        # argument for the depth rebalance in one line.
        {f"ft_split:{i}": "LEAK" for i in (1, 2, 4, 5, 8, 13, 14, 15)},
    ),
    # Off by one at the top of the alphabetic range. The reference computes
    # ((unsigned)c | 32) - 'a' < 26; widening it to 27 admits the byte just
    # past 'z' AND the byte just past 'Z', because the | 32 folds them onto
    # the same value. Only the boundary cases can see it.
    # Scan the front for characters in set, then MIRROR that count off the
    # back instead of scanning the back independently. This is the bug the
    # ft_strtrim cases could not see before 2026-08-24: every trimming case was
    # symmetric - "   hello world   " 3/3, "xxxyyy...yyyxxx" 6/6, "aabaa" 2/2 -
    # so a front-count-mirrored-to-the-back implementation scored 9/9. Verified
    # by writing one and watching it pass. Cases 2, 10, 11 and 12 now trim a
    # different number of bytes off each end.
    "strtrim_mirrors_back": (
        "ft_strtrim.c",
        "\twhile (str[end] && ft_strchr(set, str[end]))\n\t\tend--;",
        "\tend = end - start;",
        {"ft_strtrim:2": "KO", "ft_strtrim:10": "KO",
         "ft_strtrim:11": "KO", "ft_strtrim:12": "KO"},
    ),
    # The four missing-terminator mutants below all seed the same defect: the
    # allocation is one byte short, so the terminating NUL is written past the
    # end of the block. strcmp cannot see it - the text that comes back is
    # correct - and the heap usually absorbs a one-byte overflow silently.
    # Only the allocation-floor cases, which compare live_bytes against what
    # the result actually needs, tell these apart from a correct version.
    "substr_no_terminator": (
        "ft_substr.c",
        "\tsub = malloc((n + 1) * sizeof(char));",
        "\tsub = malloc(n * sizeof(char));",
        # ft_split:15 moves too: the reference builds each word with
        # ft_substr, so shorting substr's allocation shorts the split total.
        # Genuine propagation between two functions, not a second seeded bug.
        {"ft_substr:10": "KO", "ft_substr:11": "KO", "ft_substr:12": "KO",
         "ft_split:15": "KO"},
    ),
    "strjoin_no_terminator": (
        "ft_strjoin.c",
        "\tdst = malloc(sizeof(char) * l3 + 1);",
        "\tdst = malloc(sizeof(char) * l3);",
        {"ft_strjoin:8": "KO"},
    ),
    "itoa_no_terminator": (
        "ft_itoa.c",
        "\tbuffer = malloc(sizeof(char) * (size + 1));",
        "\tbuffer = malloc(sizeof(char) * size);",
        {"ft_itoa:13": "KO"},
    ),
    "strmapi_no_terminator": (
        "ft_strmapi.c",
        "\ts2 = malloc(sizeof(char) * (slen + 1));",
        "\ts2 = malloc(sizeof(char) * slen);",
        {"ft_strmapi:7": "KO"},
    ),
    # The array needs one slot per word AND the NULL that ends it. Allocating
    # only `size` slots writes that NULL one pointer past the end. Walking the
    # result never notices - the NULL reads back from memory that still holds
    # it - so the words all compare equal on a corrupted heap.
    "split_no_null_slot": (
        "ft_split.c",
        "\tnew = malloc((size + 1) * sizeof(char *));",
        "\tnew = malloc(size * sizeof(char *));",
        # ft_split:1 SIGABRTs rather than KOs: glibc's own heap checker spots
        # the overflowed array before the case can compare anything. That is
        # the corruption being caught one layer down, and the status is named
        # here so a silent change to KO would be noticed.
        {"ft_split:1": "SIGABRT", "ft_split:15": "KO"},
    ),
    "isalpha_off_by_one": (
        "ft_isalpha.c",
        "- 'a' < 26",
        "- 'a' < 27",
        # ft_isalnum:6 goes too, because the reference implements isalnum as
        # isalpha || isdigit - so the bug propagates. That is the union
        # property the spec's `why` describes, showing up as a test result.
        # ft_isalpha:10 is the full [-1, 255] sweep, so it necessarily sees
        # anything a single boundary case sees. It is listed rather than
        # excluded: a sweep that did NOT move here would mean the sweep had
        # stopped covering the boundary bytes.
        # ft_isalnum:8 is isalnum's own [-1, 255] sweep (added alongside
        # ft_isalpha's), and it inherits both the sweep coverage AND the
        # union propagation at once - confirmed by actually running this
        # mutant, not assumed from the other three.
        {"ft_isalpha:5": "KO", "ft_isalpha:7": "KO", "ft_isalpha:10": "KO",
         "ft_isalnum:6": "KO", "ft_isalnum:8": "KO"},
    ),
    # atoi that skips only ' ' instead of all six isspace characters. Exercises
    # the oracle-compared scalar path with a bug students actually write.
    # Skip only ' ' instead of all six isspace characters.
    #
    # Seeded on the loop inside ft_atoi, NOT on the mini_isspace helper above
    # it - that helper is dead code the function never calls, and patching it
    # produced a mutant that changed nothing while looking like a valid one.
    # A mutant must patch code that actually runs.
    "atoi_space_only": (
        "ft_atoi.c",
        "while (*s == 32 || (*s >= 9 && *s <= 13))",
        "while (*s == 32)",
        {"ft_atoi:8": "KO"},
    ),
    # Drop the const from ft_memcmp's first parameter - Subject IV.2 requires
    # it. Invisible from inside the project: the student's own libft.h declared
    # the same wrong signature, so everything compiled, linked and ran. It only
    # surfaces when someone includes the SUBJECT's prototype, which is what an
    # evaluator does. Caught by the prototype-conformance check, not by any
    # behavioural case - the code is functionally correct either way.
    "memcmp_drops_const": (
        [("ft_memcmp.c", "int\tft_memcmp(const void *s1",
                         "int\tft_memcmp(void *s1"),
         ("libft.h", "int\t\t\t\tft_memcmp(const void *s1",
                     "int\t\t\t\tft_memcmp(void *s1")],
        None, None,
        {},
        {"prototypes match the subject"},
    ),
    # --- macro mutants: correct C, wrong Makefile -----------------------
    # $(NAME) declared .PHONY forces a relink on every build. The classic.
    "makefile_phony_name": (
        "Makefile",
        "$(NAME):",
        ".PHONY: $(NAME)\n$(NAME):",
        {},
        {"no relink"},
    ),
    # -std=c99 reached through a variable. A grep of CFLAGS misses this;
    # reading the real compilation lines does not (decision B12).
    "makefile_hidden_c99": (
        "Makefile",
        "CFLAGS = ",
        "EXTRA = -std=c99\nCFLAGS = $(EXTRA) ",
        {},
        {"no -std=c99"},
    ),
    # Remove the INT_MIN special case and the negation overflows.
    "putnbr_no_intmin": (
        "ft_putnbr_fd.c",
        "if (n == -2147483648)",
        "if (0)",
        {"ft_putnbr_fd:7": "KO"},
    ),
    # Write one byte past what n asked for. The result and the return value
    # are both still correct - only a canary sees this, which is the whole
    # point of T1. It also breaks every ft_bzero case for free: the
    # reference's ft_bzero is a direct call to ft_memset, so the off-by-one
    # propagates through it untouched.
    "memset_extra_byte": (
        "ft_memset.c",
        "\twhile (n--)\n\t\t*dst++ = (unsigned char)c;\n\treturn (dest);\n}",
        "\twhile (n--)\n\t\t*dst++ = (unsigned char)c;\n\t*dst = (unsigned char)c;"
        "\n\treturn (dest);\n}",
        {f"ft_memset:{i}": "KO" for i in range(1, 8)}
        | {f"ft_bzero:{i}": "KO" for i in range(1, 5)},
    ),
    # size <= strlen(dst) must return size + strlen(src), because strlcat has
    # no way to know dst's real length beyond the size it was given. Using
    # the ACTUAL strlen(dst) instead is the mistake this seeds - only in the
    # early-return branch, so the normal-append cases (1-4, 9) are untouched
    # and the copy behaviour (correctly skipped) is untouched too.
    #
    # Case 5 survives by coincidence: size == strlen(dst) there (5 == 5), so
    # strlen(dst) + strlen(src) and size + strlen(src) are the same number.
    # Only 6, 7 and 8 (size < strlen(dst)) can tell the two formulas apart.
    "strlcat_wrong_formula": (
        "ft_strlcat.c",
        "\tif (l == n)\n\t\treturn (l + ft_strlen(s));",
        "\tif (l == n)\n\t\treturn (ft_strlen(d) + ft_strlen(s));",
        {"ft_strlcat:6": "KO", "ft_strlcat:7": "KO", "ft_strlcat:8": "KO"},
    ),
    # Remove the multiplication-overflow guard entirely. This is exactly the
    # case 6 / case 7 asymmetry the spec calls out: SIZE_MAX * 2 still wraps
    # to a number malloc cannot satisfy, so case 6 keeps returning NULL by
    # accident and stays green. (SIZE_MAX / 2 + 1) * 2 wraps to exactly 0,
    # so with no guard malloc(0) succeeds and hands back a live zero-byte
    # block where NULL was required - only case 7 can tell the difference,
    # which is verified by actually running this mutant.
    #
    # ft_calloc's own 64 KB cap (`65535` where the guard should read SIZE_MAX)
    # was fixed directly in the reference once found - see selftest.py's
    # REFERENCE_KNOWN_BAD history - so this mutant no longer doubles as the
    # repair of a standing baseline failure; it now seeds a clean overflow
    # check onto a clean reference, and only :7 should move.
    "calloc_no_overflow_check": (
        "ft_calloc.c",
        "\tif (n && size > SIZE_MAX / n)\n\t\treturn (NULL);\n\tnew = malloc(n * size);",
        "\tnew = malloc(n * size);",
        {"ft_calloc:7": "KO"},
        set(),
        set(),
    ),
    # Shift the index strmapi hands to the callback by one, without touching
    # the loop bounds or the terminator - every byte of the result is still
    # fully written, just with the wrong index baked in. (A first attempt
    # that started the loop COUNTER at 1 instead of 0 was rejected after
    # actually running it: on an empty string the loop body never executes,
    # so the terminator lands at s2[1] instead of s2[0], and s2[0] is read
    # back as an uninitialised, non-deterministic byte - occasionally not
    # even valid UTF-8, which crashed the reporting pipeline instead of
    # scoring KO. This version cannot leave anything uninitialised.)
    "strmapi_index_off_by_one": (
        "ft_strmapi.c",
        "\t\ts2[i] = f(i, *s);",
        "\t\ts2[i] = f(i + 1, *s);",
        {"ft_strmapi:1": "KO", "ft_strmapi:4": "KO"},
    ),
    # Drop the +1 from strdup's allocation while still copying len + 1 bytes
    # (the terminator lands one byte past the block). strcmp cannot see this
    # - cases 1-5 compare content and pointer identity only, and the heap
    # absorbs a one-byte overflow without complaining - so only the size
    # actually asked of the allocator, via cases 6-8's live-bytes check,
    # tells this apart from a correct implementation.
    "strdup_missing_terminator_byte": (
        "ft_strdup.c",
        "\td = malloc(len + 1);",
        "\td = malloc(len);",
        {"ft_strdup:6": "KO", "ft_strdup:7": "KO", "ft_strdup:8": "KO"},
    ),
    # Drop the walk-to-tail loop from ft_lstadd_back: it only ever attaches at
    # the CURRENT head's next, so a one-element list still works (head IS the
    # tail) but every longer list gets truncated - the exact bug the T5
    # handoff singled out as "never executes the walk, which is where the bug
    # actually lives". The `t_list *back` declaration has to go with it, or
    # the mutant leaves it unused under -Werror and never builds (rule 2).
    #
    # ft_lstmap:1,2,3,4,6,7 go KO as well: ft_lstmap.c appends every
    # mapped node with ft_lstadd_back, so a broken add_back silently truncates
    # the list ft_lstmap builds - genuine propagation between the two
    # functions, not a second seeded bug.
    #
    # (Those six used to be a standing LEAK baseline from a real defect in
    # ft_lstmap's rollback path. That was fixed on 2026-08-23 and reseeded as
    # lstmap_orphans_content, so they now move from OK rather than from LEAK.
    # The expected statuses are unchanged either way - check_mutants compares
    # the delta against whatever the baseline is.)
    "lstadd_back_no_walk": (
        "ft_lstadd_back.c",
        "void\tft_lstadd_back(t_list **l, t_list *new)\n{\n\tt_list\t*back;"
        "\n\n\tif (*l == NULL)\n\t{\n\t\t*l = new;\n\t\treturn ;\n\t}\n\t"
        "back = *l;\n\twhile (back -> next)\n\t\tback = back -> next;\n\t"
        "back -> next = new;\n}",
        "void\tft_lstadd_back(t_list **l, t_list *new)\n{\n\tif (*l == NULL)"
        "\n\t{\n\t\t*l = new;\n\t\treturn ;\n\t}\n\t(*l)->next = new;\n}",
        {"ft_lstadd_back:3": "KO", "ft_lstadd_back:4": "KO",
         "ft_lstadd_back:5": "KO", "ft_lstadd_back:6": "KO",
         "ft_lstmap:1": "KO", "ft_lstmap:2": "KO", "ft_lstmap:3": "KO",
         "ft_lstmap:4": "KO", "ft_lstmap:6": "KO", "ft_lstmap:7": "KO"},
    ),
    # Orphan the content f() just produced when the following ft_lstnew
    # fails: this is the real defect the reference shipped in
    # ft_lstmap.c until it was fixed on 2026-08-23 (see selftest.py's
    # REFERENCE_KNOWN_BAD history). Seeded by removing exactly the del(content)
    # line the fix added, so it restores this defect and nothing else.
    # ft_lstmap:8 moves too: dropping the call outright means del ran zero
    # times, which case 8's own allocator-arming counter catches directly -
    # it does not need the sweep at all to see this one.
    "lstmap_orphans_content": (
        "ft_lstmap.c",
        "\t\t\tdel(content);\n\t\t\tft_lstclear(&newl, del);",
        "\t\t\tft_lstclear(&newl, del);",
        {"ft_lstmap:1": "LEAK", "ft_lstmap:2": "LEAK", "ft_lstmap:3": "LEAK",
         "ft_lstmap:4": "LEAK", "ft_lstmap:6": "LEAK", "ft_lstmap:7": "LEAK",
         "ft_lstmap:8": "KO"},
    ),
    # Route the orphaned content through free() instead of del(). Every byte
    # is still released, so cases 1-7 (the BRO_INJECT sweep) see 0 bytes live
    # and a non-NULL check that never fires - free(content) satisfies both of
    # harness.c's check_injected() invariants exactly as well as del(content)
    # does, and case-level assertions made during that sweep are discarded
    # by check_injected() regardless, not just outvoted. Passed all of
    # ft_lstmap:1-315 until case 8 was added, which arms the allocator
    # itself outside BRO_INJECT specifically so its own bro_fail() is not
    # the one being discarded.
    "lstmap_free_not_del": (
        "ft_lstmap.c",
        "\t\t\tdel(content);\n\t\t\tft_lstclear(&newl, del);",
        "\t\t\tfree(content);\n\t\t\tft_lstclear(&newl, del);",
        {"ft_lstmap:8": "KO"},
    ),
    # free(content) instead of routing it through del: undetectable by a case
    # that only checks the node/content were released (free does that just as
    # well), and only visible through bro_del_counting's call count - which is
    # the entire reason that instrumented del exists instead of plain free.
    "lstdelone_free_not_del": (
        "ft_lstdelone.c",
        "\t(*del)(l -> content);\n\tfree(l);",
        "\tfree(l -> content);\n\tfree(l);",
        {"ft_lstdelone:2": "KO", "ft_lstdelone:4": "KO",
         "ft_lstdelone:5": "KO"},
    ),
    # Drop the trailing newline write: putendl becomes putstr. Every case
    # expects an exact byte count including the '\n', so all four move.
    "putendl_fd_no_newline": (
        "ft_putendl_fd.c",
        "\tft_putstr_fd(s, fd);\n\twrite(fd, \"\\n\", 1);\n}",
        "\tft_putstr_fd(s, fd);\n}",
        {"ft_putendl_fd:1": "KO", "ft_putendl_fd:2": "KO",
         "ft_putendl_fd:3": "KO", "ft_putendl_fd:4": "KO"},
    ),
}


def build(name):
    require_reference()
    if name not in MUTANTS:
        raise SystemExit(f"unknown mutant {name}")
    entry = MUTANTS[name]
    patch, old, new, expected = entry[:4]
    # A mutation may need to touch more than one file. Dropping the const from
    # ft_memcmp's parameter has to change the header too, or the two disagree
    # and the mutant does not compile - which the build gate catches, correctly.
    patches = patch if isinstance(patch, list) else [(patch, old, new)]
    macro_expect = entry[4] if len(entry) > 4 else set()
    # A mutation can legitimately RESOLVE a known baseline failure - when the
    # seeded line is the same line that carries a real defect. Declaring it
    # keeps the "baseline failures vanished" check meaningful instead of
    # forcing a weaker seed.
    fixes = entry[5] if len(entry) > 5 else set()
    dest = OUT / name
    if dest.exists():
        shutil.rmtree(dest)
    shutil.copytree(REFERENCE, dest, ignore=shutil.ignore_patterns(
        "*.o", "*.a", ".git", "*.swp"))
    for filename, o, n in patches:
        path = dest / filename
        text = path.read_text()
        if o not in text:
            raise SystemExit(
                f"{name}: seed snippet not found in {filename} - the reference "
                f"changed and this mutant needs updating")
        path.write_text(text.replace(o, n))
    subprocess.run(["make"], cwd=dest, capture_output=True)
    return dest, expected, macro_expect, fixes


def build_all(quiet=False):
    OUT.mkdir(parents=True, exist_ok=True)
    made = {}
    for name in MUTANTS:
        dest, expected, macro_expect, fixes = build(name)
        made[name] = (dest, expected, macro_expect, fixes)
        if not quiet:
            print(f"  {name:20s} {dest.relative_to(ROOT)}")
    return made


if __name__ == "__main__":
    if len(sys.argv) > 1:
        print(build(sys.argv[1])[0])
    else:
        build_all()
