"""Cache layout. Nothing ft_bro produces is written into the student's repo.

Decision A9: `make` creates .o and libft.a in their tree - that is their own
build - and everything else lives here, so `git status` stays clean and bro
cannot trip plan/platform/08-macro.md section 1's "no unused files" check with its own output.
"""

import hashlib
import os
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SPEC_DIR = ROOT / "_dev" / "plan"


def cache_root():
    base = os.environ.get("XDG_CACHE_HOME")
    if base:
        return Path(base) / "ft_bro"
    return Path.home() / ".cache" / "ft_bro"


def target_key(target):
    return hashlib.sha256(str(Path(target).resolve()).encode()).hexdigest()[:12]


def cache_dir(target):
    d = cache_root() / target_key(target)
    d.mkdir(parents=True, exist_ok=True)
    return d
