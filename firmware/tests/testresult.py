#!/usr/bin/env python3
"""Per-test result recorder for Project-civic.

Rules (set by the project owner):
- Every test has its own result file: tests/results/<test name>.result
- Every run appends exactly ONE line with a new tag and hash.
- A passing run records only "100%".
- A non-passing run records the exact reason: FAILED (exit code), CRASHED
  (signal), TIMEOUT (non-terminating / stuck), BUILD_FAILED or ERROR, plus
  the last 3 non-empty output lines of the test, joined on that one line.
- Raw test output is never printed or stored anywhere else.
- Read a result with `last <name>` (prints only the newest line), never the
  whole file.

Usage:
  testresult.py run <name> [--timeout S] [--build "CMD"] -- CMD [ARGS...]
  testresult.py pytests [--timeout S]      (one result file per test method)
  testresult.py last <name>                (newest line of one test)
  testresult.py latest                     (newest line of every test)
"""
from __future__ import annotations

import contextlib
import datetime as _dt
import hashlib
import os
import secrets
import shlex
import signal
import subprocess
import sys
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent          # firmware/tests
FIRMWARE = HERE.parent                          # firmware/
RESULTS = Path(os.environ.get("TESTRESULT_DIR", str(HERE / "results")))
TAIL_LINES = 3
LINE_MAX = 240


def _commit() -> str:
    try:
        out = subprocess.run(["git", "-C", str(FIRMWARE), "rev-parse", "--short", "HEAD"],
                             capture_output=True, text=True, timeout=10)
        return out.stdout.strip() or "nogit"
    except (OSError, subprocess.SubprocessError):
        return "nogit"


def _tail(text: str, prefer_errors: bool = False) -> str:
    lines = [ln.strip() for ln in text.splitlines() if ln.strip()]
    if prefer_errors:
        # Build failures: the compiler's "error:" diagnostics are the reason;
        # the trailing make/cc1 summary lines are not.
        diag = [ln for ln in lines if "error:" in ln and not ln.startswith(("make", "cc1"))]
        if diag:
            lines = diag
    tail = [ln[:LINE_MAX] for ln in lines[-TAIL_LINES:]]
    return " | ".join(tail) if tail else "(no output)"


def _safe_name(name: str) -> str:
    return "".join(c if c.isalnum() or c in "._-" else "_" for c in name)


def record(name: str, result: str) -> None:
    """Append one tagged line: time, tag, commit, hash, result."""
    RESULTS.mkdir(parents=True, exist_ok=True)
    now = _dt.datetime.now(_dt.timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")
    tag = "r-" + secrets.token_hex(4)
    commit = _commit()
    body = f"{now} tag={tag} commit={commit} result={result}"
    digest = hashlib.sha256(f"{name}\n{body}".encode()).hexdigest()[:12]
    line = f"{now} tag={tag} commit={commit} hash={digest} result={result}\n"
    with open(RESULTS / f"{_safe_name(name)}.result", "a", encoding="utf-8") as fh:
        fh.write(line)


SKIP_EXIT = 77                                   # exit code that means "skipped"


def _classify(rc: int | None, output: str, timed_out: bool, timeout: float, stage: str) -> str:
    prefix = "BUILD_FAILED" if stage == "build" else None
    if stage == "run" and not timed_out and rc == SKIP_EXIT:
        return f"SKIPPED :: {_tail(output)}"
    if timed_out:
        kind = f"TIMEOUT after {timeout:g}s (non-terminating or stuck)"
    elif rc is not None and rc < 0:
        try:
            sig = signal.Signals(-rc).name
        except ValueError:
            sig = f"signal {-rc}"
        kind = f"CRASHED {sig}"
    elif rc is not None and rc > 128 and rc - 128 < 65:
        try:
            sig = signal.Signals(rc - 128).name
        except ValueError:
            sig = f"signal {rc - 128}"
        kind = f"CRASHED {sig} (exit {rc})"
    else:
        kind = f"FAILED exit {rc}"
    if prefix:
        kind = f"{prefix}: {kind}"
    return f"{kind} :: {_tail(output, prefer_errors=(stage == 'build'))}"


def _execute(argv: list[str], timeout: float, env: dict | None = None):
    """Run argv; return (rc, combined output, timed_out). Output stays in memory."""
    try:
        proc = subprocess.Popen(argv, cwd=FIRMWARE, env=env, stdout=subprocess.PIPE,
                                stderr=subprocess.STDOUT, stdin=subprocess.DEVNULL,
                                start_new_session=True)
    except OSError as exc:
        return 127, f"cannot start {argv[0]}: {exc}", False
    try:
        out, _ = proc.communicate(timeout=timeout)
        return proc.returncode, out.decode("utf-8", "replace"), False
    except subprocess.TimeoutExpired:
        # communicate() can time out because a forked child inherited stdout,
        # even when the test process itself has already crashed/exited. Capture
        # the parent status before killing the process group to close orphaned
        # pipe handles, so a SIGSEGV is not misclassified as TIMEOUT.
        # A crashing process may still be writing its core dump; give it a
        # short grace period to finish dying before classifying as TIMEOUT.
        try:
            exited_rc = proc.wait(timeout=3)
        except subprocess.TimeoutExpired:
            exited_rc = None
        try:
            os.killpg(proc.pid, signal.SIGKILL)
        except ProcessLookupError:
            pass
        out, _ = proc.communicate()
        if exited_rc is not None:
            return exited_rc, out.decode("utf-8", "replace"), False
        return None, out.decode("utf-8", "replace"), True


def run_one(name: str, cmd: list[str], timeout: float, build: str | None, env: dict | None = None) -> bool:
    if build:
        rc, out, to = _execute(shlex.split(build), max(timeout, 300.0))
        if to or rc != 0:
            record(name, _classify(rc, out, to, max(timeout, 300.0), "build"))
            return False
    rc, out, to = _execute(cmd, timeout, env)
    if not to and rc == 0:
        record(name, "100%")
        return True
    record(name, _classify(rc, out, to, timeout, "run"))
    return False


def _iter_tests(suite):
    for item in suite:
        if isinstance(item, unittest.TestSuite):
            yield from _iter_tests(item)
        else:
            yield item


def run_pytests(timeout: float) -> bool:
    sys.path[:0] = [str(FIRMWARE / "tools"), str(HERE)]
    env = dict(os.environ)
    env["PYTHONPATH"] = os.pathsep.join([str(FIRMWARE / "tools"), str(HERE), env.get("PYTHONPATH", "")])
    env["PYTHONDONTWRITEBYTECODE"] = "1"
    loader = unittest.TestLoader()
    ok = True
    with open(os.devnull, "w") as sink, contextlib.redirect_stdout(sink), contextlib.redirect_stderr(sink):
        suite = loader.discover(str(HERE), pattern="test_*.py", top_level_dir=str(HERE))
    for test in _iter_tests(suite):
        test_id = test.id()
        if test_id.startswith("unittest.loader."):
            # A module failed to import: record the loader's reason under the module name.
            err = getattr(test, "_exception", None)
            name = "py." + test_id.rsplit(".", 1)[-1]
            record(name, f"ERROR import failed :: {_tail(str(err))}")
            ok = False
            continue
        cmd = [sys.executable, str(Path(__file__).resolve()), "_pyone", test_id]
        ok &= run_one("py." + test_id, cmd, timeout, None, env)
    return ok


def _pyone(test_id: str) -> int:
    """Run one unittest id. On non-success the LAST printed lines are the reason."""
    sys.path[:0] = [str(FIRMWARE / "tools"), str(HERE)]
    try:
        suite = unittest.defaultTestLoader.loadTestsFromName(test_id)
    except Exception as exc:                     # noqa: BLE001 - report any load failure
        print(f"ERROR cannot load test: {type(exc).__name__}: {exc}")
        return 1
    result = unittest.TestResult()
    suite.run(result)
    for _test, text in result.errors:
        print(text.rstrip())
        print("ERROR (exception raised in test)")
        return 1
    for _test, text in result.failures:
        print(text.rstrip())
        return 1
    for _test, reason in result.skipped:
        print(f"SKIPPED: {reason}")
        return SKIP_EXIT
    for _test in result.unexpectedSuccesses:
        print("UNEXPECTED SUCCESS (test marked expectedFailure passed)")
        return 1
    if result.testsRun != 1:
        print(f"ERROR expected 1 test, ran {result.testsRun}")
        return 1
    return 0


def last(name: str) -> int:
    path = RESULTS / f"{_safe_name(name)}.result"
    if not path.exists():
        print(f"{name}: no result recorded")
        return 1
    with open(path, "rb") as fh:                 # seek from the end: newest line only
        fh.seek(0, os.SEEK_END)
        pos = fh.tell()
        buf = b""
        while pos > 0 and buf.count(b"\n") < 2:
            step = min(4096, pos)
            pos -= step
            fh.seek(pos)
            buf = fh.read(step) + buf
    lines = [ln for ln in buf.decode("utf-8", "replace").splitlines() if ln.strip()]
    print(f"{name}: {lines[-1] if lines else 'empty'}")
    return 0


def main(argv: list[str]) -> int:
    if len(argv) < 2:
        print(__doc__)
        return 2
    mode = argv[1]
    if mode == "run":
        args = argv[2:]
        if "--" not in args or not args:
            print("usage: run <name> [--timeout S] [--build CMD] -- CMD", file=sys.stderr)
            return 2
        sep = args.index("--")
        opts, cmd = args[:sep], args[sep + 1:]
        name, timeout, build = opts[0], 30.0, None
        i = 1
        while i < len(opts):
            if opts[i] == "--timeout":
                timeout = float(opts[i + 1]); i += 2
            elif opts[i] == "--build":
                build = opts[i + 1]; i += 2
            else:
                print(f"unknown option {opts[i]}", file=sys.stderr)
                return 2
        return 0 if run_one(name, cmd, timeout, build) else 1
    if mode == "pytests":
        timeout = float(argv[3]) if len(argv) > 3 and argv[2] == "--timeout" else 60.0
        return 0 if run_pytests(timeout) else 1
    if mode == "_pyone" and len(argv) == 3:
        return _pyone(argv[2])
    if mode == "last" and len(argv) == 3:
        return last(argv[2])
    if mode == "latest":
        if not RESULTS.exists():
            print("no results recorded")
            return 1
        for path in sorted(RESULTS.glob("*.result")):
            last(path.stem)
        return 0
    print(__doc__)
    return 2


if __name__ == "__main__":
    sys.exit(main(sys.argv))
