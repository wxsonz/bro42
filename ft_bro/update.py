"""Opt-in update check (decision A18).

Deliberately NOT automatic. An update check on every run would contradict two
things already committed to - the dashboard's "nothing loads from the network"
rule (A2) and the requirement that bro work on a cluster machine with no
internet (A8) - and would put latency in front of a tool whose whole point is
fast feedback.

So: it runs only when asked, it uses git rather than an HTTP endpoint (there is
no server to run), and it fails quietly and usefully when there is no network.
"""

import subprocess

from . import VERSION, paths


def _git(*args):
    return subprocess.run(["git", "-C", str(paths.ROOT), *args],
                          capture_output=True, text=True, timeout=20)


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
    if _git("fetch", "--quiet", "origin").returncode:
        print(f"  could not reach {origin}")
        print("  (offline is fine - bro never needs the network to run)")
        return 0
    local = _git("rev-parse", "HEAD").stdout.strip()[:8]
    remote = _git("rev-parse", "@{u}").stdout.strip()[:8]
    if not remote or local == remote:
        print(f"  up to date with {origin} ({local})")
        return 0
    behind = _git("rev-list", "--count", "HEAD..@{u}").stdout.strip()
    print(f"  {behind} commit(s) behind {origin}")
    print(f"  local {local}, remote {remote}")
    print("  update with:  git pull && make")
    return 0
