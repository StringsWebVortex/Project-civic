# Project rules for AI agents (set by the project owner)

These rules apply to every agent, thread and subagent working in this repository.

## Test results
- Every test writes its own result file:
  `firmware/tests/results/<test name>.result`.
- Every run appends one line with a new tag and hash: `result=100%` on success, or the
  exact non-success reason (FAILED / CRASHED / TIMEOUT / BUILD_FAILED / ERROR / SKIPPED) followed by last three lines but if test is about coverage then coverage test should report all those file paths which found below coverage gate, coverage target/gate of this project is 99%.
- Run tests only through the recorder: `make test-host`, `make test-sanitize`,
  `make test-build`, `make test-esp32`, `make lint` (run from `firmware/`). New tests must be added to these targets or to the Python test
  discovery, so they record their own result the same way. if a test is not exist or a test is missing, but project require/deserve/needed that test then write that test.
