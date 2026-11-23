#!/usr/bin/env python3
"""Mail the board's CAN logs when the board asks for it (log >= 70 % full).

Runs on any internet-connected machine that reaches the board over USB
(host PC, or a head unit). The board keeps the request pending until this
script confirms delivery, so with no internet it simply waits and sends as
soon as the connection is back.

    python3 tools/log_mailer.py            # loop forever (default board 192.168.7.1)
    python3 tools/log_mailer.py --once     # one check, for cron / tests

Sender, recipient and the Gmail App Password are set on the dashboard
(Settings -> Log email); this script reads them from the board.
"""
import argparse
import json
import smtplib
import socket
import sys
import time
import urllib.request
from email.message import EmailMessage

BOARD = "http://192.168.7.1"
SMTP_HOST, SMTP_PORT = "smtp.gmail.com", 465


def http_get(url, timeout=10):
    with urllib.request.urlopen(url, timeout=timeout) as r:
        return r.status, r.read()


def http_post(url, timeout=10):
    req = urllib.request.Request(url, data=b"", method="POST")
    with urllib.request.urlopen(req, timeout=timeout) as r:
        return r.status, r.read()


def internet_up(host=SMTP_HOST, port=SMTP_PORT, timeout=5):
    try:
        socket.create_connection((host, port), timeout).close()
        return True
    except OSError:
        return False


def smtp_send(msg, user, password):
    with smtplib.SMTP_SSL(SMTP_HOST, SMTP_PORT, timeout=60) as s:
        s.login(user, password)
        s.send_message(msg)


def build_message(st, attachments):
    msg = EmailMessage()
    msg["From"], msg["To"] = st["from"], st["to"]
    msg["Subject"] = "Civic CAN logs - sector %d (%d.%d %% full)" % (
        st["seq"], st["fill"] // 10, st["fill"] % 10)
    msg.set_content("CAN monitor log export. Sector sequence %d.\n" % st["seq"])
    for name, data in attachments:
        msg.add_attachment(data, maintype="text", subtype="csv", filename=name)
    return msg


def check_once(board=BOARD, get=http_get, post=http_post, online=internet_up, send=smtp_send):
    """One poll. Returns 'idle', 'unconfigured', 'offline', 'sent' or 'error'."""
    code, body = get(board + "/mail/status")
    if code != 200:
        return "error"
    st = json.loads(body)
    if not st.get("pending"):
        return "idle"
    if not st.get("configured"):
        return "unconfigured"
    if not online():
        return "offline"
    files = []
    for name, path in (("civic_historical_data.csv", "/log.csv?historical=1"), ("civic_log.csv", "/log.csv")):
        try:
            c, data = get(board + path)
        except OSError:
            continue
        if c == 200 and data:
            files.append((name, data))
    send(build_message(st, files), st["from"], st["pass"])
    c, _ = post(board + "/mail/ack?seq=%d" % st["seq"])
    return "sent" if c == 200 else "error"


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--board", default=BOARD)
    ap.add_argument("--interval", type=int, default=30)
    ap.add_argument("--once", action="store_true")
    a = ap.parse_args(argv)
    while True:
        try:
            r = check_once(a.board)
        except (OSError, ValueError, smtplib.SMTPException) as e:
            r = "error: %s" % e
        print(time.strftime("%H:%M:%S"), r, flush=True)
        if a.once:
            return 0 if not r.startswith("error") else 1
        time.sleep(a.interval)


if __name__ == "__main__":
    sys.exit(main())
