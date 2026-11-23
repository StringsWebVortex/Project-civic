#!/usr/bin/env python3
"""Line coverage of the firmware sources exercised by the host C tests.

Rebuilds every C test listed in the Makefile's C_TESTS with --coverage,
runs it, and merges gcov line counts per project source file. Exits 1 and
prints every file below the gate (99 %) so the recorder stores them.

    python3 tests/coverage.py [--gate 99]
"""
import argparse
import json
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "build" / "cov"


def makefile():
    return (ROOT / "Makefile").read_text()


def c_tests(mk):
    m = re.search(r"^C_TESTS\s*=\s*(.+)$", mk, re.M)
    return m.group(1).split()


def recipe(mk, test):
    m = re.search(r"^build/%s:.*\n\tmkdir -p build\n" % re.escape(test), mk, re.M)
    if not m:
        raise SystemExit("no recipe for " + test)
    parts = []
    for ln in mk[m.end():].splitlines():
        parts.append(ln.strip().rstrip("\\").strip())
        if not ln.rstrip().endswith("\\"):
            break
    cmd = " ".join(parts)
    vars_ = {}
    for vm in re.finditer(r"^([A-Za-z_]+)\s*\??=\s*((?:.*\\\n)*.*)$", mk, re.M):
        vars_.setdefault(vm.group(1), vm.group(2).replace("\\\n", " "))
    for _ in range(5):
        cmd = re.sub(r"\$\(([A-Za-z_]+)\)", lambda v: vars_.get(v.group(1), v.group(0))
                     if v.group(1) not in ("HOST_CC", "HOST_CFLAGS") else v.group(0), cmd)
    cmd = cmd.replace("$(HOST_CFLAGS)", "-std=c11 -Wall -Wextra -pedantic")
    cmd = cmd.replace("$(HOST_CC)", "cc").replace("$@", str(OUT / test / test))
    cmd = re.sub(r"\s-O[0-3s]\b", " ", cmd).replace("-Werror", "")
    return cmd + " -O0 --coverage"


def gcov_json(gcda):
    """Yield (source path, {line: count}) from one .gcda via gcov's JSON output."""
    res = subprocess.run(["gcov", "-j", "-t", str(gcda)], cwd=ROOT, capture_output=True, text=True)
    for chunk in res.stdout.splitlines():
        chunk = chunk.strip()
        if not chunk.startswith("{"):
            continue
        for f in json.loads(chunk).get("files", []):
            yield f["file"], {l["line_number"]: l["count"] for l in f.get("lines", [])}


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--gate", type=float, default=99.0)
    a = ap.parse_args()
    mk = makefile()
    shutil.rmtree(OUT, ignore_errors=True)
    merged = {}
    for t in c_tests(mk):
        subprocess.run(["make", "-s", "build/" + t], cwd=ROOT, check=True,
                       stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        d = OUT / t
        d.mkdir(parents=True)
        # recipes use paths relative to ROOT; objects/.gcno/.gcda land next to the exe in d
        subprocess.run(recipe(mk, t), shell=True, cwd=ROOT, check=True,
                       stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        subprocess.run(str(d / t), cwd=ROOT, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        for gcda in d.glob("*.gcda"):
            for path, counts in gcov_json(gcda):
                rel = os.path.relpath(os.path.join(ROOT, path), ROOT)
                if not (rel.startswith("Core/Src/") or rel.startswith("USB/")) or not rel.endswith(".c"):
                    continue
                m = merged.setdefault(rel, {})
                for ln, n in counts.items():
                    m[ln] = m.get(ln, 0) + n
    low = []
    for f in sorted(merged):
        total = len(merged[f])
        hit = sum(1 for n in merged[f].values() if n > 0)
        pct = 100.0 * hit / total if total else 100.0
        if pct < a.gate:
            low.append("%s %.1f%%" % (f, pct))
    if low:
        # the recorder keeps the last 3 lines of <= 240 chars: pack the list
        out, cur = [], "below %.0f%% gate (%d files):" % (a.gate, len(low))
        for item in low:
            if len(cur) + len(item) + 2 > 235:
                out.append(cur)
                cur = ""
            cur += (" " if cur else "") + item + ";"
        out.append(cur)
        print("\n".join(out))
        return 1
    print("all %d files >= %.0f%%" % (len(merged), a.gate))
    return 0


if __name__ == "__main__":
    sys.exit(main())
