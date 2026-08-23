"""Run bro_micro and consume its NDJSON, joining prose as each line arrives."""

import json
import subprocess

from . import content


def run(binary, only=None, timeout_ms=2000, skip=None):
    cmd = [str(binary), "--timeout", str(timeout_ms)]
    if only:
        cmd += ["--only", only]
    if skip:
        cmd += ["--skip", ",".join(sorted(skip))]
    # errors="replace": stdout carries evidence bytes copied straight out of
    # the student's buffers, which are not required to be valid UTF-8. An
    # uninitialised byte must score the case KO, never take down the run.
    proc = subprocess.run(cmd, capture_output=True, text=True,
                          errors="replace")
    results = []
    for line in proc.stdout.splitlines():
        line = line.strip()
        if not line.startswith("{"):
            continue
        try:
            results.append(content.join(json.loads(line)))
        except json.JSONDecodeError:
            continue
    return results, proc.stderr
