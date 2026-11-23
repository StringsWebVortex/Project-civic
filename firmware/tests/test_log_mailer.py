"""Unit tests for tools/log_mailer.py (no network, no board)."""
import importlib.util
import json
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location("log_mailer", ROOT / "tools/log_mailer.py")
lm = importlib.util.module_from_spec(spec)
spec.loader.exec_module(lm)

ST = {"pending": True, "seq": 7, "sent": 6, "fill": 712, "configured": True,
      "to": "stringswebvortex@gmail.com", "from": "stringswebvortex@gmail.com", "pass": "abcd efgh"}


class Fake:
    def __init__(self, st, logs=True, ack=200):
        self.st, self.logs, self.ack, self.posts, self.sent = st, logs, ack, [], []

    def get(self, url, timeout=10):
        if url.endswith("/mail/status"):
            return 200, json.dumps(self.st).encode()
        if not self.logs:
            raise OSError("no log")
        return 200, b"ts,key\n1,ON\n"

    def post(self, url, timeout=10):
        self.posts.append(url)
        return self.ack, b""

    def send(self, msg, user, pw):
        self.sent.append((msg, user, pw))


def run(f, online=True):
    return lm.check_once("http://b", f.get, f.post, lambda: online, f.send)


class LogMailerTests(unittest.TestCase):
    def test_sends_and_acks_when_pending(self):
        f = Fake(dict(ST))
        self.assertEqual(run(f), "sent")
        msg, user, pw = f.sent[0]
        self.assertEqual(msg["To"], "stringswebvortex@gmail.com")
        self.assertIn("71.2 % full", msg["Subject"])
        self.assertEqual(len(list(msg.iter_attachments())), 2)
        self.assertEqual((user, pw), (ST["from"], "abcd efgh"))
        self.assertEqual(f.posts, ["http://b/mail/ack?seq=7"])

    def test_waits_for_internet(self):
        f = Fake(dict(ST))
        self.assertEqual(run(f, online=False), "offline")
        self.assertEqual(f.sent, [])
        self.assertEqual(f.posts, [])

    def test_idle_and_unconfigured(self):
        self.assertEqual(run(Fake(dict(ST, pending=False))), "idle")
        self.assertEqual(run(Fake(dict(ST, configured=False))), "unconfigured")

    def test_missing_logs_still_sends_and_bad_ack(self):
        f = Fake(dict(ST), logs=False, ack=409)
        self.assertEqual(run(f), "error")
        self.assertEqual(len(list(f.sent[0][0].iter_attachments())), 0)

    def test_status_error(self):
        self.assertEqual(lm.check_once("http://b", lambda u, timeout=10: (500, b""), None, None, None), "error")

    def test_internet_up_false_on_unreachable(self):
        self.assertFalse(lm.internet_up("127.0.0.1", 9, 0.2))

    def test_main_once_reports_error_without_board(self):
        self.assertEqual(lm.main(["--once", "--board", "http://127.0.0.1:9"]), 1)


if __name__ == "__main__":
    unittest.main()
