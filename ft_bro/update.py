"""Opt-in update check (decision A18), and the passive notice it leaves behind.

Deliberately NOT automatic. An update check on every run would contradict two
things already committed to - the dashboard's "nothing loads from the network"
rule (A2) and the requirement that bro work on a cluster machine with no
internet (A8) - and would put latency in front of a tool whose whole point is
fast feedback.

So the network call and the notice are separate things:

  check()    talks to the remote. Runs only when asked, via --check-update,
             and writes what it learned to paths.update_cache().
  pending()  reads that file. No network, no subprocess, no failure mode
             beyond "there is nothing cached". Every run calls this, and the
             terminal footer and the dashboard show the result.

A student who never runs --check-update is never nagged and never delayed.
One who runs it once keeps seeing the notice until they actually update.

Releases, not commits (B33). A tagged version is a deliberate act; a typo fix
pushed to main is not, and should not tell everyone they are out of date.
"""

import json
import re
import subprocess
import time

from . import VERSION, paths


def _git(*args):
    return subprocess.run(["git", "-C", str(paths.ROOT), *args],
                          capture_output=True, text=True, timeout=20,
                          errors="replace")


def _parse(tag):
    """'v1.2.3' -> (1, 2, 3). Anything unparseable sorts lowest."""
    m = re.match(r"v?(\d+)\.(\d+)\.(\d+)", tag or "")
    return tuple(int(g) for g in m.groups()) if m else (0, 0, 0)


def _write(**fields):
    path = paths.update_cache()
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps({"checked": time.time(), **fields}))


def pending():
    """The cached verdict, or None. Never touches the network.

    Returns the newer version string only while it is still newer than what is
    running - so updating makes the notice disappear on its own, without
    needing the cache to be cleared or re-checked.
    """
    try:
        data = json.loads(paths.update_cache().read_text())
    except (OSError, ValueError):
        return None
    latest = data.get("latest")
    if latest and _parse(latest) > _parse(VERSION):
        return latest
    return None


def check():
    print(f"ft_bro {VERSION}")
    if not (paths.ROOT / ".git").exists():
        print("  not a git checkout - nothing to compare against")
        return 0
    r = _git("remote", "get-url", "origin")
    if r.returncode:
        print("  no origin remote configured")
        return 0
    origin = r.stdout.strip()

    # ls-remote, not fetch: a check should not modify the repository it is
    # checking. Fetching would also mean reading LOCAL tags afterwards, so a
    # stray v* tag someone made by hand would report an update that does not
    # exist. This asks the remote and believes only the remote.
    r = _git("ls-remote", "--tags", "--refs", "origin", "v*")
    if r.returncode:
        print(f"  could not reach {origin}")
        print("  (offline is fine - bro never needs the network to run)")
        return 0

    tags = [line.split("refs/tags/")[-1]
            for line in r.stdout.splitlines() if "refs/tags/" in line]
    latest = max(tags, key=_parse, default="")
    if not latest:
        print(f"  {origin} has published no release tags yet")
        _write(latest=None, origin=origin)
        return 0

    _write(latest=latest.lstrip("v"), origin=origin)
    if _parse(latest) <= _parse(VERSION):
        print(f"  up to date - {origin} is at {latest}")
        return 0

    print(f"  {latest} is available (you have {VERSION})")
    print("  update with:  git -C %s pull && make" % paths.ROOT)
    return 0
