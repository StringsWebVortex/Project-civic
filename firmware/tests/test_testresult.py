"""Checks that the per-test result recorder writes the right single line."""
import os, subprocess, sys, tempfile, unittest
from pathlib import Path

RECORDER = Path(__file__).resolve().parent / "testresult.py"


class RecorderTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.env = dict(os.environ, TESTRESULT_DIR=self.tmp.name)

    def tearDown(self):
        self.tmp.cleanup()

    def run_rec(self, name, *cmd, timeout="5"):
        subprocess.run([sys.executable, str(RECORDER), "run", name, "--timeout", timeout, "--", *cmd],
                       env=self.env, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, timeout=60)
        lines = (Path(self.tmp.name) / f"{name}.result").read_text().splitlines()
        return lines

    def test_pass_records_100_percent(self):
        lines = self.run_rec("ok", sys.executable, "-c", "print('noise')")
        self.assertEqual(len(lines), 1)
        self.assertTrue(lines[0].endswith("result=100%"))
        self.assertNotIn("noise", lines[0])

    def test_failure_records_exit_and_last_three_lines(self):
        code = "import sys\nfor i in range(6): print('line', i)\nsys.exit(3)"
        line = self.run_rec("bad", sys.executable, "-c", code)[-1]
        self.assertIn("result=FAILED exit 3 :: line 3 | line 4 | line 5", line)
        self.assertNotIn("line 2", line)

    def test_build_failure_records_compiler_errors(self):
        code = ("print('a.c:1:1: error: first problem')\n"
                "print('   | ^~~~')\nprint('cc1: all warnings being treated as errors')\n"
                "print('make: *** [x] Error 1')\nimport sys; sys.exit(2)")
        subprocess.run([sys.executable, str(RECORDER), "run", "b", "--build",
                        f"{sys.executable} -c \"{code}\"", "--", "true"],
                       env=self.env, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, timeout=60)
        line = (Path(self.tmp.name) / "b.result").read_text().splitlines()[-1]
        self.assertIn("BUILD_FAILED: FAILED exit 2 :: a.c:1:1: error: first problem", line)
        self.assertNotIn("cc1", line)

    def test_crash_records_signal(self):
        line = self.run_rec("crash", sys.executable, "-c", "import os,signal; os.kill(os.getpid(), signal.SIGSEGV)")[-1]
        self.assertIn("result=CRASHED SIGSEGV", line)

    def test_hang_records_timeout(self):
        line = self.run_rec("hang", sys.executable, "-c", "import time; time.sleep(30)", timeout="1")[-1]
        self.assertIn("result=TIMEOUT after 1s", line)

    def test_skip_exit_records_skipped(self):
        code = "import sys\nprint('SKIPPED: node not installed')\nsys.exit(77)"
        line = self.run_rec("skip", sys.executable, "-c", code)[-1]
        self.assertIn("result=SKIPPED :: SKIPPED: node not installed", line)
        self.assertNotIn("FAILED", line)

    def test_each_run_appends_new_tag_and_hash(self):
        self.run_rec("again", "true")
        lines = self.run_rec("again", "true")
        self.assertEqual(len(lines), 2)
        tags = [l.split(" tag=")[1].split()[0] for l in lines]
        hashes = [l.split(" hash=")[1].split()[0] for l in lines]
        self.assertNotEqual(tags[0], tags[1])
        self.assertNotEqual(hashes[0], hashes[1])


if __name__ == "__main__":
    unittest.main()
