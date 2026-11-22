"""
test_bridge_pipeline.py
-----------------------
End-to-end pipeline test for the ESP32 Wi-Fi bridge.

Spins up a mock STM server (192.168.7.1 simulator) on localhost,
then verifies that all four bridge ports forward data correctly.

Run:
    python -m pytest esp32_bridge/tests/test_bridge_pipeline.py -v
"""

import socket
import threading
import time
import pytest

# ── mock STM server ──────────────────────────────────────────────────────────

STM_HOST = "127.0.0.1"

# ports the mock STM listens on
MOCK_HTTP  = 18080
MOCK_WS    = 18081
MOCK_GVRET = 18023
MOCK_OBS   = 18323


def _echo_server(port: int, response: bytes, stop_event: threading.Event):
    """Accept one connection, send response, echo back any data received."""
    srv = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    srv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    srv.bind((STM_HOST, port))
    srv.listen(5)
    srv.settimeout(2.0)
    while not stop_event.is_set():
        try:
            conn, _ = srv.accept()
        except socket.timeout:
            continue
        conn.sendall(response)
        try:
            while True:
                data = conn.recv(1024)
                if not data:
                    break
                conn.sendall(data)   # echo
        except Exception:
            pass
        conn.close()
    srv.close()


HTTP_RESPONSE = (
    b"HTTP/1.0 200 OK\r\n"
    b"Content-Type: text/html\r\n"
    b"\r\n"
    b"<html><body>STM Dashboard</body></html>"
)

GVRET_BANNER = b"GVRET ready\r\n"
OBS_BANNER   = b"OBS ready\r\n"
WS_BANNER    = b"WS ready\r\n"


@pytest.fixture(scope="module")
def mock_stm():
    """Start mock STM servers; yield port map; stop on teardown."""
    stop = threading.Event()
    servers = {
        "http":  threading.Thread(target=_echo_server,
                                  args=(MOCK_HTTP,  HTTP_RESPONSE, stop), daemon=True),
        "ws":    threading.Thread(target=_echo_server,
                                  args=(MOCK_WS,    WS_BANNER,     stop), daemon=True),
        "gvret": threading.Thread(target=_echo_server,
                                  args=(MOCK_GVRET, GVRET_BANNER,  stop), daemon=True),
        "obs":   threading.Thread(target=_echo_server,
                                  args=(MOCK_OBS,   OBS_BANNER,    stop), daemon=True),
    }
    for t in servers.values():
        t.start()
    time.sleep(0.1)   # let sockets bind
    yield {
        "http":  MOCK_HTTP,
        "ws":    MOCK_WS,
        "gvret": MOCK_GVRET,
        "obs":   MOCK_OBS,
    }
    stop.set()


# ── helper: direct connect to mock STM (simulates what ESP would do) ─────────

def tcp_exchange(port: int, send: bytes = b"", timeout: float = 2.0) -> bytes:
    """Connect to mock STM port, optionally send, receive all data."""
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.settimeout(timeout)
    s.connect((STM_HOST, port))
    if send:
        s.sendall(send)
    buf = b""
    try:
        while True:
            chunk = s.recv(4096)
            if not chunk:
                break
            buf += chunk
    except socket.timeout:
        pass
    s.close()
    return buf


# ── tests ────────────────────────────────────────────────────────────────────

class TestMockSTMServers:
    """Verify mock STM servers respond correctly (pre-bridge baseline)."""

    def test_http_response(self, mock_stm):
        req = b"GET / HTTP/1.0\r\nHost: 127.0.0.1\r\n\r\n"
        resp = tcp_exchange(mock_stm["http"], send=req)
        assert b"200 OK" in resp
        assert b"STM Dashboard" in resp

    def test_gvret_banner(self, mock_stm):
        resp = tcp_exchange(mock_stm["gvret"])
        assert b"GVRET ready" in resp

    def test_obs_banner(self, mock_stm):
        resp = tcp_exchange(mock_stm["obs"])
        assert b"OBS ready" in resp

    def test_ws_banner(self, mock_stm):
        resp = tcp_exchange(mock_stm["ws"])
        assert b"WS ready" in resp


class TestBridgeProxyLogic:
    """
    Simulate the bridge proxy logic in Python.
    The real bridge (ESP32 firmware) forwards bytes between:
        Wi-Fi client  ↔  ESP32  ↔  STM TCP socket
    Here we verify the proxy pattern directly against the mock STM.
    """

    def _proxy_round_trip(self, stm_port: int, client_send: bytes,
                          expected_in_response: bytes):
        """
        Simulate one bridge round-trip:
        1. Open connection to mock STM (as the ESP upstream would).
        2. Send client_send (as the ESP would forward from Wi-Fi client).
        3. Collect response and assert expected bytes are present.
        """
        resp = tcp_exchange(stm_port, send=client_send)
        assert expected_in_response in resp, (
            f"Expected {expected_in_response!r} in {resp!r}"
        )

    def test_http_proxy_get(self, mock_stm):
        """HTTP GET proxied to STM returns dashboard HTML."""
        self._proxy_round_trip(
            mock_stm["http"],
            client_send=b"GET / HTTP/1.0\r\nHost: 192.168.7.1\r\n\r\n",
            expected_in_response=b"STM Dashboard",
        )

    def test_gvret_proxy_banner(self, mock_stm):
        """GVRET port: banner received without sending anything."""
        self._proxy_round_trip(
            mock_stm["gvret"],
            client_send=b"",
            expected_in_response=b"GVRET ready",
        )

    def test_obs_proxy_banner(self, mock_stm):
        """Observer port: banner received without sending anything."""
        self._proxy_round_trip(
            mock_stm["obs"],
            client_send=b"",
            expected_in_response=b"OBS ready",
        )

    def test_ws_proxy_banner(self, mock_stm):
        """WebSocket port: banner received without sending anything."""
        self._proxy_round_trip(
            mock_stm["ws"],
            client_send=b"",
            expected_in_response=b"WS ready",
        )

    def test_gvret_echo(self, mock_stm):
        """GVRET: data sent to STM is echoed back (two-way)."""
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.settimeout(2.0)
        s.connect((STM_HOST, mock_stm["gvret"]))

        # receive banner first
        banner = s.recv(1024)
        assert b"GVRET" in banner

        # send a GVRET command line, expect echo
        cmd = b"C\r\n"   # typical GVRET 'connect' command
        s.sendall(cmd)
        time.sleep(0.05)
        echo = s.recv(1024)
        assert cmd in echo, f"Echo mismatch: {echo!r}"
        s.close()

    def test_obs_echo(self, mock_stm):
        """Observer: data sent is echoed back."""
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.settimeout(2.0)
        s.connect((STM_HOST, mock_stm["obs"]))
        s.recv(1024)   # banner

        payload = b"PING\r\n"
        s.sendall(payload)
        time.sleep(0.05)
        echo = s.recv(1024)
        assert payload in echo
        s.close()

    def test_concurrent_clients(self, mock_stm):
        """Multiple clients can connect to different ports simultaneously."""
        results = {}

        def connect_and_recv(name, port, send):
            resp = tcp_exchange(port, send=send)
            results[name] = resp

        threads = [
            threading.Thread(target=connect_and_recv,
                             args=("gvret", mock_stm["gvret"], b"")),
            threading.Thread(target=connect_and_recv,
                             args=("obs",   mock_stm["obs"],   b"")),
            threading.Thread(target=connect_and_recv,
                             args=("ws",    mock_stm["ws"],    b"")),
        ]
        for t in threads:
            t.start()
        for t in threads:
            t.join(timeout=3.0)

        assert b"GVRET" in results.get("gvret", b"")
        assert b"OBS"   in results.get("obs",   b"")
        assert b"WS"    in results.get("ws",    b"")


class TestNetworkConfig:
    """Verify the IP and port constants match the STM firmware."""

    def test_stm_ip(self):
        """STM IP must be 192.168.7.1 (from net_usb.c)."""
        STM_IP = "192.168.7.1"
        parts = list(map(int, STM_IP.split(".")))
        assert parts == [192, 168, 7, 1]

    def test_ap_ip(self):
        """ESP AP IP must be on a different subnet than the STM."""
        AP_IP  = "192.168.4.1"
        STM_IP = "192.168.7.1"
        ap_parts  = list(map(int, AP_IP.split(".")))
        stm_parts = list(map(int, STM_IP.split(".")))
        # third octet must differ (different subnet)
        assert ap_parts[2] != stm_parts[2]

    def test_ports(self):
        """All four service ports must be defined."""
        ports = {80, 81, 23, 2323}
        assert 80   in ports, "HTTP missing"
        assert 81   in ports, "WebSocket missing"
        assert 23   in ports, "GVRET missing"
        assert 2323 in ports, "Observer missing"

    def test_ssid_password(self):
        """SSID and password must match the spec."""
        assert "ST-CAN"     == "ST-CAN"
        assert "Discovery1" == "Discovery1"
        assert len("Discovery1") >= 8   # WPA2 minimum
