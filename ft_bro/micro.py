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
    proc = subprocess.run(cmd, capture_output=True, text=True)
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
