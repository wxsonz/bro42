"""`bro --serve` (plan/platform/07-dashboard.md).

A local dashboard server whose only purpose is to give the ⚡ Re-run buttons
something to talk to - everything else the dashboard shows works from the
inlined report.html with no server at all (decision A2). stdlib only
(decision B10): no Flask, no bundler, nothing to `pip install`.

Bound to 127.0.0.1 only - this is a tool for the student's own machine, not a
service, and it must never be reachable from the network.

    GET  /            the same report.html `bro` would open
    GET  /api/report  the current report.json
    POST /api/run     re-run everything, or {"filter": "ft_split"} for one
                       function; returns the updated report.json
    POST /api/macro   re-run the macro (build) suite only; returns the
                       updated report.json

The pack is fixed for the life of one `bro --serve` process - it is whatever
`bro` detected (or was told with --pack) when the server started, same as
every other subcommand.
"""

import json
import sys
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer

from . import build, content, history, macro, micro, paths, report

HOST = "127.0.0.1"
PORT = 4242


def _existing_report(target, pack):
    p = paths.cache_dir(target, pack) / "report.json"
    if p.is_file():
        try:
            return json.loads(p.read_text())
        except json.JSONDecodeError:
            return None
    return None


def _flatten(data):
    """The stored report's suites, flattened back into engine-shaped records,
    so a scoped re-run can be merged back into the rest of a full report
    without re-running every suite to redraw one of them."""
    out = []
    for s in data.get("suites", []) if data else []:
        out.extend(s.get("cases", []))
    return out


def _run_records(target, pack, only=None):
    """One function (only=fn) or the whole suite (only=None). Never raises -
    mirrors bro42/cli.py's own handling of a repo that fails to build."""
    binary, info = build.prepare(target, pack)
    if binary is None:
        return None, info.get("error", "cannot build this target")
    skip = list(info.get("missing") or []) + list(info.get("blocked") or {})
    records, _err = micro.run(binary, pack, only=only, skip=skip)
    content.annotate_missing(pack, records, info)
    return records, None


def do_run(target, pack, filt=None):
    """Mirrors cli.py's `restricted` distinction (plan/platform/04-testdesign.md /
    03_ORCHESTRATOR.md): selecting one function is a view over a full run,
    not a suite of its own, so it does not get to redefine history."""
    from .cli import resolve_suite

    existing = _existing_report(target, pack)
    checks = existing.get("macro") if existing else None

    if filt:
        fn = resolve_suite(pack, filt)
        fresh, error = _run_records(target, pack, only=fn)
        if fresh is None:
            return None, error
        records = [r for r in _flatten(existing) if r.get("fn") != fn] + fresh
        report.write(records, target, pack, checks=checks, hist_delta=None)
    else:
        records, error = _run_records(target, pack)
        if records is None:
            return None, error
        entries = history.read(target, pack)[-60:]
        entry = history.summarise(records, checks)
        previous = entries[-1] if entries else None
        hist_delta = history.delta(previous, entry, entries + [entry])
        history.append(target, pack, entry)
        report.write(records, target, pack, checks=checks, hist_delta=hist_delta)

    return _existing_report(target, pack), None


def do_macro(target, pack):
    checks = [c.as_dict() for c in macro.audit(target, pack)]
    existing = _existing_report(target, pack)
    records = _flatten(existing)
    if not records:
        # No prior run to attach these checks to - seed one so the page has
        # something to show alongside the build results.
        records, error = _run_records(target, pack)
        if records is None:
            return None, error
    report.write(records, target, pack, checks=checks, hist_delta=None)
    return _existing_report(target, pack), None


class Handler(BaseHTTPRequestHandler):
    server_version = "bro42-serve/1"

    # The terminal is this tool's voice, not an HTTP access log.
    def log_message(self, fmt, *args):
        pass

    def _target(self):
        return self.server.bro_target

    def _pack(self):
        return self.server.bro_pack

    def _json(self, code, obj):
        body = json.dumps(obj).encode()
        self.send_response(code)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def _html(self, path):
        if not path.is_file():
            return self._json(404, {"error": "no report yet - run bro once first"})
        body = path.read_bytes()
        self.send_response(200)
        self.send_header("Content-Type", "text/html; charset=utf-8")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def _body(self):
        try:
            length = int(self.headers.get("Content-Length") or 0)
        except ValueError:
            length = 0
        if not length:
            return {}
        raw = self.rfile.read(length)
        try:
            obj = json.loads(raw) if raw.strip() else {}
            return obj if isinstance(obj, dict) else {}
        except json.JSONDecodeError:
            return {}

    def do_GET(self):
        target, pack = self._target(), self._pack()
        cache = paths.cache_dir(target, pack)
        if self.path in ("/", "/index.html", "/report.html"):
            self._html(cache / "report.html")
        elif self.path == "/api/report":
            p = cache / "report.json"
            if p.is_file():
                self._json(200, json.loads(p.read_text()))
            else:
                self._json(404, {"error": "no report yet - run bro once first"})
        else:
            self._json(404, {"error": "not found"})

    def do_POST(self):
        target, pack = self._target(), self._pack()
        if self.path == "/api/run":
            try:
                data, error = do_run(target, pack, self._body().get("filter"))
            except SystemExit as e:
                data, error = None, str(e)
        elif self.path == "/api/macro":
            data, error = do_macro(target, pack)
        else:
            return self._json(404, {"error": "not found"})
        if data is None:
            self._json(500, {"error": error})
        else:
            self._json(200, data)


def serve(target, pack):
    httpd = ThreadingHTTPServer((HOST, PORT), Handler)
    httpd.bro_target = target
    httpd.bro_pack = pack
    print(f"  bro42 serving http://{HOST}:{PORT}   (bound to localhost only, Ctrl+C to stop)")
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        print()
    finally:
        httpd.server_close()
    return 0


def main(target, pack):
    if not (paths.cache_dir(target, pack) / "report.json").is_file():
        print(f"  bro42: no report yet for {target} - running once to seed it")
        data, error = do_run(target, pack, None)
        if data is None:
            sys.stderr.write(f"bro: cannot serve {target}\n  {error}\n")
            return 126
    return serve(target, pack)
