#!/usr/bin/env python3
"""On-board self-test over the dashboard WebSocket (no transceiver needed).

For each bus: switch to LOOPBACK_SILENT (bus pins stay recessive, TJA1051 S
HIGH), send a standard and an extended frame, require both to come back as
received frames, then return to SILENT. Also checks write mode is OFF at
boot, the device/ability commands answer, GVRET reports both buses, and
(on Linux) the host bound cdc_ether to a CDC-ECM (subclass 06) interface.

    python3 tools/board_selftest.py [--host 192.168.7.1]
Exit 0 = all checks passed; otherwise prints the failed checks.
"""
import argparse
import base64
import json
import os
import socket
import struct
import sys
import time


class WS:
    def __init__(self, host, port=81, timeout=3.0):
        self.s = socket.create_connection((host, port), timeout)
        key = base64.b64encode(os.urandom(16)).decode()
        self.s.sendall(("GET / HTTP/1.1\r\nHost: %s\r\nUpgrade: websocket\r\nConnection: Upgrade\r\n"
                        "Sec-WebSocket-Key: %s\r\nSec-WebSocket-Version: 13\r\n\r\n" % (host, key)).encode())
        buf = b""
        while b"\r\n\r\n" not in buf:
            buf += self.s.recv(1024)
        if b" 101 " not in buf.split(b"\r\n", 1)[0]:
            raise OSError("websocket upgrade refused")
        self.rest = buf.split(b"\r\n\r\n", 1)[1]

    def send(self, obj):
        p = json.dumps(obj).encode()
        mask = os.urandom(4)
        hdr = bytes([0x81]) + (bytes([0x80 | len(p)]) if len(p) < 126 else bytes([0x80 | 126]) + struct.pack(">H", len(p)))
        self.s.sendall(hdr + mask + bytes(b ^ mask[i % 4] for i, b in enumerate(p)))

    def _read(self, n):
        while len(self.rest) < n:
            chunk = self.s.recv(4096)
            if not chunk:
                raise OSError("closed")
            self.rest += chunk
        out, self.rest = self.rest[:n], self.rest[n:]
        return out

    def recv(self):
        b0, b1 = self._read(2)
        n = b1 & 0x7F
        if n == 126:
            n = struct.unpack(">H", self._read(2))[0]
        elif n == 127:
            n = struct.unpack(">Q", self._read(8))[0]
        data = self._read(n)
        return (b0 & 0x0F), data

    def messages(self, seconds):
        end = time.time() + seconds
        while time.time() < end:
            self.s.settimeout(max(0.05, end - time.time()))
            try:
                op, data = self.recv()
            except socket.timeout:
                return
            if op == 1:
                try:
                    yield json.loads(data)
                except ValueError:
                    pass


def texts(msgs):
    out = []
    for m in msgs:
        out.append(json.dumps(m))
    return "\n".join(out)


def gvret_params(host):
    s = socket.create_connection((host, 23), 3)
    s.settimeout(1.5)
    s.sendall(b"\xe7\xe7")
    time.sleep(0.2)
    s.sendall(b"\xf1\x06")
    time.sleep(0.4)
    r = s.recv(64)
    s.close()
    return r


def host_ecm_driver():
    """Linux only: driver bound to the board's ECM control interface (0483:5740)."""
    base = "/sys/bus/usb/devices"
    if not os.path.isdir(base):
        return None
    for d in os.listdir(base):
        p = os.path.join(base, d)
        try:
            if open(os.path.join(p, "idVendor")).read().strip() != "0483" or \
               open(os.path.join(p, "idProduct")).read().strip() != "5740":
                continue
        except OSError:
            continue
        for i in os.listdir(p):
            ip = os.path.join(p, i)
            if ":1.0" in i and os.path.exists(os.path.join(ip, "bInterfaceSubClass")):
                sub = open(os.path.join(ip, "bInterfaceSubClass")).read().strip()
                drv = os.path.basename(os.path.realpath(os.path.join(ip, "driver"))) if os.path.exists(os.path.join(ip, "driver")) else ""
                return sub, drv
    return None


def main(argv=None):
    ap = argparse.ArgumentParser()
    ap.add_argument("--host", default="192.168.7.1")
    a = ap.parse_args(argv)
    fails = []
    ws = WS(a.host)
    ws.send({"op": "can", "line": "WRITE,?"})
    ws.send({"op": "can", "line": "DEVICE,STATUS"})
    ws.send({"op": "can", "line": "OPT,F"})
    ws.send({"op": "can", "line": "ERRSTATE,B"})
    boot = texts(ws.messages(2.0))
    for want in ("STATUS,SYS,WRITE,OFF", "DEVICE,", "STATUS,F,OPTS,ABOM=1,AWUM=1,NART=0,RFLM=0,TXFP=0,TTCM=1,DBF=1,SJW=2",
                 "STATUS,B,ERRSTATE,"):
        if want not in boot:
            fails.append("missing " + want)
    for bus in ("F", "B"):
        ws.send({"op": "mode", "bus": bus, "mode": "LOOPBACKSILENT"})
        time.sleep(0.3)
        ws.send({"op": "tx", "bus": bus, "format": "S", "id": "123", "rtr": 0, "dlc": 2, "data": "A55A"})
        ws.send({"op": "tx", "bus": bus, "format": "E", "id": "18DAF110", "rtr": 0, "dlc": 3, "data": "010203"})
        got = texts(ws.messages(2.0))
        if "MODE,LOOPBACKSILENT" not in got:
            fails.append(bus + ": loopback mode not confirmed")
        if "TX,%s,ERR" % bus in got or "NOT_ARMED" in got:
            fails.append(bus + ": loopback TX refused")
        if "123" not in got or "A55A" not in got.upper():
            fails.append(bus + ": standard frame not looped back")
        if "18DAF110" not in got.upper():
            fails.append(bus + ": extended frame not looped back")
        ws.send({"op": "mode", "bus": bus, "mode": "SILENT"})
        time.sleep(0.3)
    ecm = host_ecm_driver()
    if ecm is not None and ecm != ("06", "cdc_ether"):
        fails.append("USB interface is not CDC-ECM/cdc_ether: %r" % (ecm,))
    r = gvret_params(a.host)
    if len(r) < 12 or r[2] != 0x11 or r[7] != 0x11:
        fails.append("GVRET bus params wrong: " + r.hex())
    if fails:
        print("FAILED: " + "; ".join(fails))
        return 1
    print("board self-test: all checks passed")
    return 0


if __name__ == "__main__":
    sys.exit(main())
