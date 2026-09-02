"""Cache layout. Nothing bro42 produces is written into the student's repo.

Decision A9: `make` creates .o and libft.a in their tree - that is their own
build - and everything else lives here, so `git status` stays clean and bro
cannot trip plan/platform/08-macro.md section 1's "no unused files" check with its own output.
"""

import hashlib
import os
import shutil
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SPEC_DIR = ROOT / "_dev" / "plan"

# The project was called ft_bro through 1.0.1; Phase 0 renamed it to bro42.
# The OLD name is kept here on purpose - see _migrate() below - so a
# student's existing cache is found and moved rather than orphaned.
_OLD_NAME = "ft_bro"
_NEW_NAME = "bro42"


def _cache_root_named(name):
    base = os.environ.get("XDG_CACHE_HOME")
    if base:
        return Path(base) / name
    return Path.home() / ".cache" / name


def cache_root():
    return _cache_root_named(_NEW_NAME)


def _old_cache_root():
    return _cache_root_named(_OLD_NAME)


def target_key(target):
    return hashlib.sha256(str(Path(target).resolve()).encode()).hexdigest()[:12]


def _migrate(old, new, what):
    """Move `old` to `new`, once, if `new` is not there yet but `old` is.

    Decision A10: run history is what makes this a companion rather than a
    scoreboard - the since-last-run delta and the streak live only in this
    directory. Renaming the project must not silently orphan weeks of a
    student's history, so the old cache root is checked once and moved over.

    Must never raise: a permission error or a race with another `bro`
    process degrades to "no history for this run", never to a crashed run.
    A failed migration is not a failed test run.
    """
    try:
        if new.exists() or not old.exists():
            return
        new.parent.mkdir(parents=True, exist_ok=True)
        shutil.move(str(old), str(new))
        print(f"bro: moved your {what} from {old} to {new}", file=sys.stderr)
    except Exception:
        pass


def _migrate_flat_to_pack(flat_dir, pack_dir):
    """Phase 2 nested a pack id under the directory that used to be the leaf
    itself: `<root>/<sha>/` became `<root>/<sha>/<pack>/`. A cache left flat
    at `<root>/<sha>/` predates packs entirely - bro42 tested only Libft
    before this release - so it IS the Libft pack's history, not a sibling of
    it, and has to become `pack_dir`'s contents rather than be orphaned next
    to them.

    `flat_dir` and `pack_dir` are nested (`pack_dir` is `flat_dir / pack.id`),
    so this cannot be a plain shutil.move like _migrate's - moving a
    directory into its own future subdirectory is not something shutil
    supports. Stage through a sibling temp name instead.

    Must never raise, same contract as _migrate: a failed migration is not a
    failed test run.
    """
    try:
        if pack_dir.exists() or not flat_dir.exists():
            return
        # Only a leaf that actually looks like run history is flat legacy -
        # a directory that already holds pack subdirectories (this
        # migration's own prior work, or a fresh multi-pack cache with
        # nothing in it yet) must be left alone.
        if not any((flat_dir / name).exists()
                  for name in ("history.jsonl", "report.json")):
            return
        tmp = flat_dir.with_name(flat_dir.name + ".migrating")
        if tmp.exists():
            shutil.rmtree(tmp)
        shutil.move(str(flat_dir), str(tmp))
        shutil.move(str(tmp), str(pack_dir))
        print(f"bro: moved your run history from {flat_dir} to {pack_dir}",
              file=sys.stderr)
    except Exception:
        pass


def cache_dir(target, pack):
    key = target_key(target)
    new_root = cache_root()
    old_root = _old_cache_root()
    d = new_root / key / pack.id

    # Both migrations only ever produce a LIBFT history: the ft_bro name and
    # the flat <sha>/ layout both predate every pack but Libft, so neither
    # can be a plausible source of another pack's cache.
    if pack.id == "libft":
        _migrate(old_root / key, d, "run history")
        _migrate_flat_to_pack(new_root / key, d)
    _migrate(old_root / "update.json", new_root / "update.json", "update check")

    d.mkdir(parents=True, exist_ok=True)
    return d


def update_cache():
    """Where `bro --check-update` leaves what it learned.

    Kept at the cache ROOT, not under a target: the answer is about bro42
    itself, so checking it once from one libft should tell you about every
    other one.
    """
    return cache_root() / "update.json"
