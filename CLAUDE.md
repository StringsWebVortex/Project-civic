# Project rules for AI agents (set by the project owner)

These rules apply to every agent, thread and subagent working in this repository.

## Local only
- Never push to any remote. Keep everything local; save work as git commits.

## Test results
- Every test writes its own result file: `firmware/tests/results/<test name>.result`.
- Every run appends one line with a new tag and hash: `result=100%` on success, or the
  exact non-success reason (FAILED / CRASHED / TIMEOUT / BUILD_FAILED / ERROR / SKIPPED)
  followed by the last 3 output lines.
- Run tests only through the recorder: `make test-host`, `make test-sanitize`,
  `make test-build`, `make test-esp32`, `make lint` (run from `firmware/`). New tests must be added to these targets or to the Python test
  discovery, so they record their own result the same way.
- Agents must NOT read raw test output, logs, reports or metadata, and must NOT read a
  whole result file. Read only the newest line: `make result T=<name>` or `make results`.
  Do not bypass this (no `cat`/`tail -n 2+`/`-v` runs/direct test binaries).
