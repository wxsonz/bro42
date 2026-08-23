"""The macro suite: everything about a Libft that is not a function's behaviour.

Checks come from _dev/SPEC_MACRO.md, which states WHAT is asserted and cites the
subject clause. The methods here are design/08_MACRO.md's, and three of them are
deliberately not the obvious ones:

  B12  flags are read from the real compilation command lines
       (`make --always-make -n`), not grepped out of the Makefile. A grep misses
       CC used through a variable, CFLAGS composed across assignments, and a
       -std=c99 hidden inside $(EXTRA).
  B13  relinking is MEASURED by .o mtimes, not inferred from "Nothing to be
       done" - that string is locale-dependent, differs between GNU and BSD
       make, and is trivially produced by a Makefile that is in fact relinking.
  B17  everything runs against a copy in the cache. `make fclean` deletes the
       student's libft.a, and an interrupted run would leave them with none.

Severity is part of the result (B18): `fail` requires a subject citation,
`warn` says what evaluation will think, `skip` names a missing tool.
"""

import re
import shutil
import subprocess
import time
from pathlib import Path

from . import content, libft, paths

COPY_PATTERNS = ("Makefile", "makefile", "*.c", "*.h", "*.md")

# Captured tool output is displayed in a terminal AND inside a <pre> in the
# dashboard. norminette colours its findings, and those escapes render as
# literal "[97m" garbage in HTML.
ANSI_RE = re.compile(r"\x1b\[[0-9;]*[A-Za-z]")


def strip_ansi(text):
    return ANSI_RE.sub("", text)


class Check:
    def __init__(self, section, name, status, desc, sev="fail",
                 command="", output="", hint=""):
        self.section = section
        self.name = name
        self.status = status      # OK | FAIL | WARN | SKIP
        self.desc = desc
        self.sev = sev
        self.command = command
        self.output = output
        self.hint = hint

    def as_dict(self):
        return {
            "section": self.section, "name": self.name, "status": self.status,
            "desc": self.desc, "sev": self.sev, "command": self.command,
            "output": strip_ansi(self.output)[-4000:], "hint": self.hint,
        }


def run(cmd, cwd, timeout=60):
    try:
        return subprocess.run(cmd, cwd=cwd, capture_output=True, text=True,
                              timeout=timeout)
    except subprocess.TimeoutExpired:
        return subprocess.CompletedProcess(cmd, 124, "", "timed out")


def make_copy(target):
    """B17: never run destructive make targets in the student's own tree."""
    dest = paths.cache_dir(target) / "target-copy"
    if dest.exists():
        shutil.rmtree(dest)
    dest.mkdir(parents=True)
    for pattern in COPY_PATTERNS:
        for src in Path(target).glob(pattern):
            if src.is_file():
                shutil.copy2(src, dest / src.name)
    return dest


def tracked_artifacts(root):
    r = run(["git", "ls-files", "*.o", "*.a"], root)
    if r.returncode:
        return []
    return [l.strip() for l in r.stdout.splitlines() if l.strip()]


def objects(d):
    return sorted(d.glob("*.o"))


def mtimes(d):
    return {p.name: p.stat().st_mtime_ns for p in objects(d)}


# ---------------------------------------------------------------- section 1

def check_structure(target, out):
    root = Path(target)
    for name, why in (("Makefile", "Subject IV.1: all files at the root"),
                      ("libft.h", "Subject IV: Files to Submit")):
        ok = (root / name).is_file()
        out.append(Check(1, f"{name} present", "OK" if ok else "FAIL",
                         f"{name} {'found' if ok else 'missing'} at the root",
                         hint=why))

    missing = [f for f in libft.MANDATORY if not (root / f"{f}.c").is_file()]
    out.append(Check(
        1, "mandatory sources", "OK" if not missing else "FAIL",
        f"{len(libft.MANDATORY) - len(missing)}/{len(libft.MANDATORY)} present",
        hint=("not written yet: " + ", ".join(missing[:8])) if missing else
             "Subject IV: every mandatory function has its own .c file"))

    bonus = [f for f in libft.PART3
             if (root / f"{f}.c").is_file() or (root / f"{f}_bonus.c").is_file()]
    non_std = [f for f in bonus if (root / f"{f}.c").is_file()]
    if not bonus:
        out.append(Check(1, "bonus files", "WARN", "Part 3 not implemented",
                         sev="warn", hint="Part 3 is optional"))
    elif non_std:
        out.append(Check(1, "bonus files", "WARN",
                         f"{len(non_std)} bonus file(s) not named _bonus.c",
                         sev="warn",
                         hint="Subject II: bonuses go in _bonus.{c/h} files"))
    else:
        out.append(Check(1, "bonus files", "OK",
                         f"{len(bonus)} bonus function(s), correctly named"))

    known = {f"{f}.c" for f in libft.ALL} | {f"{f}_bonus.c" for f in libft.ALL}
    stray = sorted(p.name for p in root.glob("*.c") if p.name not in known)
    # Object files at the root are what `make` produces and `make clean`
    # removes - flagging them on every run would be crying wolf. What the
    # subject actually forbids is COMMITTING them, so only tracked ones count.
    stray += sorted(tracked_artifacts(root))
    out.append(Check(
        1, "no stray files", "OK" if not stray else "WARN",
        "clean" if not stray else f"{len(stray)} extra: " + ", ".join(stray[:6]),
        sev="warn",
        hint="Subject IV.1: submitting unused files is not allowed - a main.c "
             "at the root also breaks anything that links your library"))


# ---------------------------------------------------------------- section 2

def check_rules(work, out):
    r = run(["make", "re"], work)
    built = (work / "libft.a").is_file()
    out.append(Check(2, "make / all", "OK" if built else "FAIL",
                     "libft.a built" if built else "no libft.a after make",
                     command="make re", output=r.stdout + r.stderr,
                     hint="Subject II: rules $(NAME), all, clean, fclean, re"))
    if not built:
        return False

    run(["make", "clean"], work)
    left = objects(work)
    lib = (work / "libft.a").is_file()
    ok = not left and lib
    out.append(Check(2, "make clean", "OK" if ok else "FAIL",
                     "objects removed, libft.a kept" if ok else
                     f"{len(left)} .o left, libft.a {'kept' if lib else 'DELETED'}",
                     command="make clean",
                     hint="clean removes intermediate objects only"))

    run(["make", "fclean"], work)
    ok = not objects(work) and not (work / "libft.a").is_file()
    out.append(Check(2, "make fclean", "OK" if ok else "FAIL",
                     "objects and libft.a removed" if ok else "something survived",
                     command="make fclean",
                     hint="fclean is clean plus the build product"))

    run(["make"], work)
    r = run(["make", "bonus"], work)
    if r.returncode:
        out.append(Check(2, "make bonus", "WARN", "no working bonus rule",
                         sev="warn", command="make bonus",
                         output=r.stdout + r.stderr,
                         hint="Subject II: bonuses need a bonus rule"))
    else:
        syms = archive_symbols(work / "libft.a")
        present = [f for f in libft.PART3 if f in syms]
        ok = len(present) == len(libft.PART3)
        out.append(Check(
            2, "make bonus", "OK" if ok else "WARN",
            f"{len(present)}/{len(libft.PART3)} bonus symbols in the archive",
            sev="warn", command="make bonus",
            hint="a bonus rule that does nothing satisfies a grep - the "
                 "symbols are the only real test"))
    return True


# ---------------------------------------------------------------- section 3

def check_relink(work, out):
    run(["make"], work)
    before = mtimes(work)
    lib_before = (work / "libft.a").stat().st_mtime_ns
    time.sleep(0.01)
    r = run(["make"], work)
    after = mtimes(work)
    moved = [k for k in after if before.get(k) != after[k]]
    relinked = (work / "libft.a").stat().st_mtime_ns != lib_before
    ok = not moved and not relinked
    out.append(Check(
        3, "no relink", "OK" if ok else "FAIL",
        "a second make rebuilds nothing" if ok else
        f"{len(moved)} object(s) recompiled"
        + (" and libft.a was re-created" if relinked else ""),
        command="make (twice, comparing .o mtimes)", output=r.stdout,
        hint="Subject II: no unnecessary relinking. The usual cause is "
             "$(NAME) declared .PHONY"))

    sources = sorted(work.glob("*.c"))
    if not sources:
        return
    before = mtimes(work)
    time.sleep(0.01)
    sources[0].touch()
    run(["make"], work)
    after = mtimes(work)
    moved = [k for k in after if before.get(k) != after[k]]
    ok = len(moved) == 1
    out.append(Check(
        3, "incremental build", "OK" if ok else "FAIL",
        f"touching {sources[0].name} recompiled {len(moved)} object(s)",
        command=f"touch {sources[0].name} && make",
        hint="exactly one object should rebuild - a Makefile that rebuilds "
             "everything is not relinking, but it is still wrong, and the "
             "'up to date' test cannot see it"))

    header = work / "libft.h"
    if header.is_file():
        before = mtimes(work)
        time.sleep(0.01)
        header.touch()
        run(["make"], work)
        after = mtimes(work)
        moved = [k for k in after if before.get(k) != after[k]]
        out.append(Check(
            3, "header dependency", "OK" if moved else "WARN",
            f"touching libft.h recompiled {len(moved)} object(s)",
            sev="warn", command="touch libft.h && make",
            hint="without the header as a prerequisite, changing a struct "
                 "leaves stale objects that disagree with it"))


# ---------------------------------------------------------------- section 4

def compile_lines(work):
    r = run(["make", "--always-make", "-n"], work)
    lines = []
    for line in (r.stdout + r.stderr).splitlines():
        if re.search(r"(?<![\w/])(cc|gcc|clang|g\+\+)\b", line) and " -c " in line:
            lines.append(line.strip())
    return lines, r


def check_flags(work, out):
    lines, r = compile_lines(work)
    if not lines:
        out.append(Check(4, "compiler & flags", "SKIP",
                         "could not read the compilation commands",
                         sev="skip", command="make --always-make -n",
                         output=r.stdout + r.stderr))
        return

    wrong = [l for l in lines
             if not re.search(r"(?<![\w/])cc\b", l.split(" -c ")[0])]
    out.append(Check(4, "compiler is cc", "OK" if not wrong else "FAIL",
                     "cc" if not wrong else f"{len(wrong)} line(s) use another compiler",
                     command="make --always-make -n",
                     output="\n".join(wrong[:3]),
                     hint="Subject II: compile with cc"))

    for flag in ("-Wall", "-Wextra", "-Werror"):
        bad = [l for l in lines if flag not in l]
        out.append(Check(4, f"{flag}", "OK" if not bad else "FAIL",
                         "on every compile line" if not bad else
                         f"missing from {len(bad)}/{len(lines)} line(s)",
                         command="make --always-make -n",
                         output="\n".join(bad[:3]),
                         hint="Subject IV.1: every .c must compile with "
                              "-Wall -Wextra -Werror - every line, not the first"))

    c99 = [l for l in lines if "-std=c99" in l]
    out.append(Check(4, "no -std=c99", "OK" if not c99 else "FAIL",
                     "absent" if not c99 else f"present on {len(c99)} line(s)",
                     command="make --always-make -n",
                     output="\n".join(c99[:2]),
                     hint="Subject IV.2 note: -std=c99 is forbidden"))

    text = (work / "Makefile").read_text() if (work / "Makefile").is_file() else ""
    out.append(Check(4, "no libtool", "OK" if "libtool" not in text else "FAIL",
                     "not used" if "libtool" not in text else "libtool invoked",
                     hint="Subject IV.1: libtool is strictly forbidden"))

    run(["make", "fclean"], work)
    r = run(["make"], work)
    warnings = [l for l in (r.stdout + r.stderr).splitlines()
                if "warning:" in l or "error:" in l]
    out.append(Check(4, "builds clean", "OK" if not warnings else "FAIL",
                     "0 warnings" if not warnings else
                     f"{len(warnings)} diagnostic(s)",
                     command="make fclean && make",
                     output="\n".join(warnings[:10]),
                     hint="under -Werror a warning is already an error"))


def check_prototypes(target, work, out):
    """Do the sources match the prototypes Subject IV.2 specifies?

    This is invisible from inside the project. A student whose libft.h declares
    `int ft_memcmp(void *s1, ...)` - no const - compiles and links and runs
    perfectly, because their own header agrees with their own source and C does
    not check parameter types across translation units. It breaks the moment
    anyone includes the subject's prototype instead, which is what an
    evaluator's main does.

    So: compile each source against a header generated from the prototypes in
    SPEC_MICRO.md, with the student's own header out of the include path, and
    report the conflicts.
    """
    from . import build

    probe = work.parent / "protocheck"
    if probe.exists():
        shutil.rmtree(probe)
    probe.mkdir(parents=True)
    sources = [p for p in Path(target).glob("*.c")
               if p.stem in libft.ALL or p.stem.replace("_bonus", "") in libft.ALL]
    if not sources:
        return
    for src in sources:
        shutil.copy2(src, probe / src.name)
    build.synth_header(probe)

    from . import content
    protos = {n: m.get("prototype", "")
              for n, m in content.load()["functions"].items()}

    bad = []
    for src in sorted(probe.glob("*.c")):
        r = run(["cc", "-fsyntax-only", str(src), f"-I{probe}"], probe)
        for line in r.stderr.splitlines():
            if "conflicting types" not in line:
                continue
            fn = src.stem.replace("_bonus", "")
            # "have 'int(void *, const void *, size_t)' {aka '...'}" - the aka
            # is the same thing with typedefs expanded, pure noise here.
            have = line.split("have ", 1)[-1].split("{aka", 1)[0]
            have = have.strip().strip("'\u2018\u2019")
            # gcc's type notation is not what they wrote. Find the actual
            # declaration in their source - that is the line to go and edit.
            for src_line in strip_ansi(src.read_text()).splitlines():
                if re.match(rf"^\s*[\w*\s]+\b{fn}\s*\(", src_line) \
                        and ";" not in src_line:
                    have = " ".join(src_line.split())
                    break
            # Showing only what they wrote is useless: it is what they already
            # believe. The actionable half is the signature the subject asks
            # for, side by side with it.
            bad.append((fn, protos.get(fn, "(not in the spec)"), have))
    detail = "\n".join(
        f"{fn}\n    subject:  {want}\n    yours:    {have}"
        for fn, want, have in bad)
    out.append(Check(
        4, "prototypes match the subject", "OK" if not bad else "FAIL",
        "all match" if not bad
        else ", ".join(f for f, _, _ in bad)
        + (" differs" if len(bad) == 1 else f" differ ({len(bad)})"),
        output=detail,
        hint="Subject IV.2 gives the exact prototypes. Your own libft.h can "
             "declare a different one and everything still compiles for you - "
             "the mismatch only appears when someone includes the subject's "
             "header, which is what an evaluator does."))


# ---------------------------------------------------------------- sections 5,6

def archive_symbols(archive):
    r = run(["nm", "-g", "--defined-only", str(archive)], archive.parent)
    return {p.split()[-1].lstrip("_")
            for p in r.stdout.splitlines() if len(p.split()) >= 3
            and p.split()[-2] in ("T", "t")}


def check_symbols(work, out):
    archive = work / "libft.a"
    if not archive.is_file():
        out.append(Check(5, "symbol audit", "SKIP", "no libft.a to inspect",
                         sev="skip"))
        return

    r = run(["nm", "-g", "--defined-only", str(archive)], work)
    globals_found = []
    for line in r.stdout.splitlines():
        parts = line.split()
        if len(parts) >= 3 and parts[-2] in ("D", "B", "C", "G", "S"):
            globals_found.append(parts[-1])
    out.append(Check(
        5, "no global variables", "OK" if not globals_found else "FAIL",
        "none" if not globals_found else ", ".join(globals_found[:5]),
        command="nm -g --defined-only libft.a",
        hint="Subject IV.1: declaring global variables is strictly forbidden. "
             "File-scope statics are lowercase in nm and are perfectly legal - "
             "this check must not confuse the two"))

    r = run(["nm", "-u", str(archive)], work)
    obj = None
    offenders = {}
    ft_names = set(libft.ALL) | {f"{f}_bonus" for f in libft.ALL}
    for line in r.stdout.splitlines():
        line = line.strip()
        if line.endswith(".o:"):
            obj = line[:-1]
            continue
        parts = line.split()
        if len(parts) != 2 or parts[0] != "U" or obj is None:
            continue
        sym = parts[1].lstrip("_").split("@")[0]
        if sym.startswith("ft_") or sym in ft_names:
            continue
        fn = obj[:-2].replace("_bonus", "")
        part = libft.PART_OF.get(fn, 2)
        if sym not in libft.ALLOWED_EXTERNALS[part]:
            offenders.setdefault(obj, []).append(sym)
    if offenders:
        detail = "; ".join(f"{k}: {', '.join(v)}" for k, v in
                           list(offenders.items())[:4])
    else:
        detail = "only the permitted externals"
    out.append(Check(
        5, "external functions", "OK" if not offenders else "FAIL", detail,
        command="nm -u libft.a",
        hint="Subject IV.2/IV.3/IV.4: Part 1 and 3 may use malloc and free, "
             "Part 2 may also use write. Calls between your own ft_* "
             "functions are legal"))


def check_archive(work, out):
    archive = work / "libft.a"
    if not archive.is_file():
        out.append(Check(6, "archive", "SKIP", "no libft.a", sev="skip"))
        return
    r = run(["file", str(archive)], work)
    ok = "archive" in r.stdout
    out.append(Check(6, "valid ar archive", "OK" if ok else "FAIL",
                     r.stdout.split(":", 1)[-1].strip()[:60],
                     command="file libft.a",
                     hint="Subject IV.1: you must use ar to create your library"))

    syms = archive_symbols(archive)
    expected = [f for f in libft.MANDATORY if (work / f"{f}.c").is_file()]
    missing = [f for f in expected if f not in syms]
    out.append(Check(
        6, "symbols exported", "OK" if not missing else "FAIL",
        f"{len(expected) - len(missing)}/{len(expected)} exported as T",
        command="nm -g --defined-only libft.a",
        hint=("in the archive but not exported: " + ", ".join(missing[:6]))
             if missing else "each function must be callable by other programs"))


# ---------------------------------------------------------------- section 7

def check_norm(work, out):
    if not shutil.which("norminette"):
        out.append(Check(
            7, "norminette", "SKIP", "not installed", sev="skip",
            hint="norminette is the 42 code-style checker. Not scored here.\n"
                 "install:  pip3 install --user norminette"))
        return
    files = [str(p.name) for p in sorted(work.glob("*.c"))]
    files += [str(p.name) for p in sorted(work.glob("*.h"))]
    r = run(["norminette", "-R", "CheckForbiddenSourceHeader"] + files, work)
    errors = [l for l in r.stdout.splitlines() if "Error" in l]
    out.append(Check(
        7, "norminette", "OK" if not errors else "FAIL",
        "0 errors" if not errors else f"{len(errors)} error line(s)",
        command="norminette -R CheckForbiddenSourceHeader *.c *.h",
        output="\n".join(errors[:15]),
        hint="Subject II: a norm error is a zero, bonus files included"))


# ---------------------------------------------------------------- section 8

README_SECTIONS = ("Description", "Instructions", "Resources")


def check_readme(target, out):
    path = Path(target) / "README.md"
    if not path.is_file():
        out.append(Check(8, "README.md", "FAIL", "missing",
                         hint="Subject V: a README.md at the root of your repository"))
        return
    text = path.read_text()
    first = next((l for l in text.splitlines() if l.strip()), "")
    ok = bool(re.match(r"^\s*[*_].*42 curriculum.*[*_]\s*$", first))
    out.append(Check(8, "first line", "OK" if ok else "FAIL",
                     first[:70] if first else "(empty)",
                     hint="Subject V: the first line must be italicised and read "
                          "*This project has been created as part of the 42 "
                          "curriculum by <login>.*"))
    for name in README_SECTIONS:
        found = re.search(rf"^#{{1,6}}\s*{name}|^\*\*{name}\*\*", text,
                          re.M | re.I)
        out.append(Check(8, f"{name} section", "OK" if found else "FAIL",
                         "present" if found else "not found",
                         hint=f"Subject V requires a '{name}' section"))
    ai = re.search(r"\bAI\b|artificial intelligence|LLM|ChatGPT|Claude",
                   text, re.I)
    out.append(Check(
        8, "AI usage disclosed", "OK" if ai else "FAIL",
        "mentioned" if ai else "no mention of AI usage",
        hint="Subject V: the Resources section must describe how AI was used, "
             "for which tasks and which parts. This is the one everyone misses"))


# ---------------------------------------------------------- pitfalls scanner
#
# SPEC_LEARNING.md's "Common pitfalls (P1-P5)", implemented per
# design/08_MACRO.md's "Pitfalls scanner" section. These are heuristics over
# the student's own source, not build checks: every one of them can produce
# a false positive, so every finding is sev="warn" (never scored, see
# render.py - only "FAIL" checks ever count toward an exit code), phrased as
# a question rather than a verdict, and named the file/line it came from.
#
# Conservative by construction: each scan is scoped as narrowly as the table
# in SPEC_LEARNING.md describes it (a named function, a specific file), so a
# heuristic that is merely broad cannot fire on code the narrower rule was
# never about. Tested against _dev/reference/libft42git, which is clean -
# see the generator/selftest notes for what that run actually showed.

PITFALL_SECTION = 9


class Finding:
    def __init__(self, rule, path, line, question, concept):
        self.rule = rule
        self.path = path
        self.line = line
        self.question = question
        self.concept = concept

    def as_check(self):
        card = content.concept(self.concept)
        hint = f"concept: {self.concept}"
        if card.get("title"):
            hint += " - " + card["title"]
        return Check(
            PITFALL_SECTION, f"{self.rule} · {self.path.name}:{self.line}",
            "WARN", self.question, sev="warn", hint=hint)


def _strip_comments(text):
    """Commented-out or example code must not be scanned - a demonstration
    snippet in a header comment is not a pitfall in the student's code."""
    text = re.sub(r"/\*.*?\*/", " ", text, flags=re.S)
    text = re.sub(r"//[^\n]*", "", text)
    return text


def _line_of(text, pos):
    return text.count("\n", 0, pos) + 1


def _find_calls(text, name):
    """Balanced-paren argument text for every call to `name(`."""
    out = []
    for m in re.finditer(rf"(?<![\w]){name}\s*\(", text):
        depth = 1
        i = m.end()
        while i < len(text) and depth:
            if text[i] == "(":
                depth += 1
            elif text[i] == ")":
                depth -= 1
            i += 1
        out.append((m.start(), text[m.end():i - 1]))
    return out


def _split_signature(sig):
    """`"void ft_lstclear(t_list **lst, void (*del)(void *))"` -> ("ft_lstclear",
    "t_list **lst, void (*del)(void *)"). Split on the FIRST '(' rather than a
    single non-nested regex, because a function-pointer parameter (`void
    (*del)(void *)`) puts more parens inside the parameter list - a pattern
    that is common in this project (`del`, `f`) and must not defeat the split."""
    paren = sig.find("(")
    tail = sig.rstrip()
    if paren == -1 or not tail.endswith(")"):
        return None, None
    before = sig[:paren].rstrip()
    m = re.search(r"(\w+)$", before)
    if not m:
        return None, None
    return m.group(1), sig[paren + 1:len(tail) - 1]


def _iter_named_functions(text):
    """Yields (name, params, body, start_idx) for each brace-delimited
    block whose opening brace is preceded by a `name(params)` signature.
    Good enough for 42-Norm-shaped C without a real parser - which is all
    five heuristics need: which function a line lives in, and its parameter
    names."""
    i, n = 0, len(text)
    while i < n:
        if text[i] != "{":
            i += 1
            continue
        j = i - 1
        while j >= 0 and text[j] in " \t\r\n":
            j -= 1
        k = j
        depth = 0
        while k >= 0:
            c = text[k]
            if c == ")":
                depth += 1
            elif c == "(":
                depth -= 1
            elif c in ";{}" and depth <= 0:
                break
            k -= 1
        name, params = _split_signature(text[k + 1:j + 1])
        depth = 1
        e = i + 1
        while e < n and depth:
            if text[e] == "{":
                depth += 1
            elif text[e] == "}":
                depth -= 1
            e += 1
        if name:
            yield name, params, text[i:e], i
        i = e


def scan_p1(path, text):
    """P1: `malloc(strlen(...))` with no `+ 1` anywhere in the same
    expression - the byte for the terminator everyone forgets."""
    out = []
    for pos, arg in _find_calls(text, "malloc"):
        if not re.search(r"(?<![\w])(?:ft_)?strlen\s*\(", arg):
            continue
        if re.search(r"\+\s*1(?!\d)", arg) or re.search(r"(?<!\d)1\s*\+", arg):
            continue
        out.append(Finding("P1", path, _line_of(text, pos),
                           "is this missing the +1 for the terminator?",
                           "null-terminator"))
    return out


def scan_p2(path, text):
    """P2: a write through a `char *` that was initialised from a string
    literal - `char *s = "hi"; s[0] = ...` faults into read-only memory.
    Catches both the combined `char *s = "hi";` form and a plain `s = "hi";`
    reassignment - a bare identifier can only take a string literal if it is
    already some flavour of `char *`, so no type declaration is needed to
    know the write that follows is unsafe."""
    out = []
    for _name, _sig, body, start in _iter_named_functions(text):
        for lm in re.finditer(r"(?<![\w])(\w+)\s*=\s*\"(?:[^\"\\]|\\.)*\"\s*;", body):
            var = lm.group(1)
            after = body[lm.end():]
            write = (re.search(rf"(?<![\w]){var}\s*\[[^\]]*\]\s*=(?!=)", after)
                    or re.search(rf"\*\s*{var}\b\s*=(?!=)", after))
            if write:
                out.append(Finding(
                    "P2", path, _line_of(text, start + lm.end() + write.start()),
                    f"this writes into read-only memory - did you mean "
                    f"`char {var}[]`?", "string-literal-ro"))
    return out


def scan_p3(path, text):
    """P3: byte subtraction in a *comparison* function with no
    `unsigned char` anywhere in it. Scoped to functions whose name contains
    "cmp" (SPEC_LEARNING.md: "in a comparison function") and to dereference/
    index subtraction specifically, so plain pointer arithmetic like
    `end - start` - a different, legitimate idea - never matches."""
    out = []
    for name, _sig, body, start in _iter_named_functions(text):
        if "cmp" not in name.lower() or "unsigned char" in body:
            continue
        m = (re.search(r"\*\s*-{0,2}\s*\w+\s*-\s*\*\s*-{0,2}\s*\w+", body)
             or re.search(r"\w+\s*\[[^\]]*\]\s*-\s*\w+\s*\[[^\]]*\]", body))
        if m:
            out.append(Finding(
                "P3", path, _line_of(text, start + m.start()),
                "are you comparing chars where you meant bytes?",
                "signed-char"))
    return out


def scan_p4(path, text):
    """P4: in ft_strtrim, a loop that asks `ft_strchr(set, str[i])` without
    checking `str[i]` itself FIRST is not guarded against `set == ""` -
    strchr also matches a string's own terminator, so with an empty `set`
    it "finds" the trimmed string's terminator too, and the index walks
    past it instead of stopping."""
    if path.name != "ft_strtrim.c":
        return []
    out = []
    for m in re.finditer(r"while\s*\(((?:[^()]|\([^()]*\))*)\)", text):
        cond = m.group(1)
        sc = re.search(
            r"\bft_strchr\s*\(\s*set\s*,\s*(\w+(?:\s*\[[^\]]*\])?)\s*\)", cond)
        if not sc:
            continue
        idx_expr = re.sub(r"\s+", "", sc.group(1))
        before = re.sub(r"\s+", "", cond[:sc.start()])
        if idx_expr in before:
            continue
        out.append(Finding("P4", path, _line_of(text, m.start()),
                           'can this loop terminate when set is ""?',
                           "null-terminator"))
    return out


def scan_p5(path, text):
    """P5: `ft_lstclear` with no assignment to `*lst` - the caller is left
    holding a pointer into memory that was just freed."""
    out = []
    for name, sig, body, start in _iter_named_functions(text):
        if name != "ft_lstclear":
            continue
        pm = re.search(r"\w+\s*\*\*\s*(\w+)", sig)
        if not pm:
            continue
        param = pm.group(1)
        # A mid-loop `*lst = tmp;` (advancing the cursor) does not free the
        # caller from holding a dangling pointer - only setting it back to
        # NULL/0 does, so that is specifically what has to be present.
        if re.search(rf"\*\s*{param}\b\s*=(?!=)\s*(?:NULL|0)\b", body):
            continue
        out.append(Finding(
            "P5", path, _line_of(text, start),
            f"the caller is left holding a freed pointer - should this end "
            f"with *{param} = NULL?", "double-pointer"))
    return out


PITFALL_SCANS = (scan_p1, scan_p2, scan_p3, scan_p4, scan_p5)


def scan_pitfalls(work):
    out = []
    for path in sorted(Path(work).glob("*.c")):
        text = _strip_comments(path.read_text(errors="replace"))
        for scan in PITFALL_SCANS:
            out += [f.as_check() for f in scan(path, text)]
    return out


# ---------------------------------------------------------------- driver

def audit(target):
    out = []
    check_structure(target, out)
    work = make_copy(target)
    if (work / "Makefile").is_file():
        if check_rules(work, out):
            check_relink(work, out)
            check_flags(work, out)
            check_prototypes(target, work, out)
            check_symbols(work, out)
            check_archive(work, out)
    else:
        out.append(Check(2, "Makefile rules", "SKIP", "no Makefile to run",
                         sev="skip"))
    check_norm(work, out)
    check_readme(target, out)
    out += scan_pitfalls(work)
    return out
