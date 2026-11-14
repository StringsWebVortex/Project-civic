"""Dashboard UI check: runs tests/dashboard_harness.js in headless Chromium.

Skipped when Node.js or the 'playwright' Node package is not installed.
"""
import os
import shutil
import subprocess
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))


def _playwright_available():
    node = shutil.which("node")
    if not node:
        return False
    r = subprocess.run([node, "-e", "require('playwright')"], capture_output=True,
                       cwd=os.path.dirname(HERE))
    return r.returncode == 0


class DashboardUiTest(unittest.TestCase):
    @unittest.skipUnless(_playwright_available(), "node + playwright not installed")
    def test_dashboard_diagnostics_ui(self):
        r = subprocess.run(["node", os.path.join(HERE, "dashboard_harness.js")],
                           capture_output=True, text=True, timeout=120)
        lines = (r.stdout + r.stderr).strip().splitlines()
        self.assertEqual(r.returncode, 0, "\n".join(lines[-3:]))


if __name__ == "__main__":
    unittest.main()
