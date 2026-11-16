#!/usr/bin/env python3
"""Generates docs/schematic_civic_can_stm32f407_tja1051.svg.

Every pin number and connection on the sheet comes from an official document:
  * STM32F407G-DISC1 (MB997 rev E-01): UM1472 Table 7 (P1/P2 header pins),
    UM1472 7.4 (5V pin as input), MB997 schematic sheet 7 (R29/R33 4.7 k on
    PB6/PB9), DS8626 (FT pins, AF9 CAN, AF4 I2C1).
  * NXP TJA1051 product data sheet: pinout (Fig. 2, TJA1051T), Table 7 levels,
    Fig. 18 typical application.
  * Microchip 24LC512 data sheet DS20001754: Table 2-1 pins, 2.4 WP.
  * SAE J1962 / ISO 15765-4 diagnostic connector: 4 chassis GND, 5 signal GND,
    6 CAN-H, 14 CAN-L, 16 battery +.
Run:  python3 tools/gen_schematic.py
"""
from pathlib import Path

W, H = 2640, 1500
out = []
INK, NET, PWR, GNDC, NOTE, PIN = "#111", "#0b5cad", "#b3261e", "#1b6e20", "#555", "#6a1b9a"


def esc(t):
    return t.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;")


def text(x, y, t, size=13, anchor="start", color=INK, weight="normal", italic=False):
    st = ' font-style="italic"' if italic else ""
    out.append(f'<text x="{x}" y="{y}" font-size="{size}" text-anchor="{anchor}" fill="{color}" '
               f'font-weight="{weight}"{st}>{esc(t)}</text>')


def line(pts, color=INK, w=2):
    d = " ".join(f"{x},{y}" for x, y in pts)
    out.append(f'<polyline points="{d}" fill="none" stroke="{color}" stroke-width="{w}" stroke-linejoin="round"/>')


def dot(x, y, color=INK):
    out.append(f'<circle cx="{x}" cy="{y}" r="4" fill="{color}"/>')


def box(x, y, w, h, fill="#fffdf2", color=INK, dash=None):
    ds = f' stroke-dasharray="{dash}"' if dash else ""
    out.append(f'<rect x="{x}" y="{y}" width="{w}" height="{h}" fill="{fill}" stroke="{color}" stroke-width="2"{ds}/>')


def netlabel(x, y, name, side="right", color=NET):
    """Net-label flag at a wire end. Same name = same net."""
    w = 9 * len(name) + 16
    if side == "right":
        pts = [(x, y), (x + 8, y - 10), (x + w, y - 10), (x + w, y + 10), (x + 8, y + 10), (x, y)]
        tx = x + 12
    else:
        pts = [(x, y), (x - 8, y - 10), (x - w, y - 10), (x - w, y + 10), (x - 8, y + 10), (x, y)]
        tx = x - w + 4
    line(pts, color, 1.5)
    text(tx, y + 5, name, 12, color=color, weight="bold")


def gnd(x, y):
    line([(x, y), (x, y + 12)], GNDC)
    for i, half in enumerate((14, 9, 4)):
        line([(x - half, y + 12 + 5 * i), (x + half, y + 12 + 5 * i)], GNDC)
    text(x + 18, y + 24, "GND", 11, color=GNDC)


def rail(x, y, name, color=PWR):
    line([(x, y), (x, y - 14)], color)
    line([(x - 12, y - 14), (x + 12, y - 14)], color, 2.5)
    text(x, y - 20, name, 12, "middle", color, "bold")


def resistor(x1, y1, x2, y2, ref, val):
    """Vertical (x1 == x2) or horizontal resistor between two points."""
    if x1 == x2:
        m = (y1 + y2) / 2
        line([(x1, y1), (x1, m - 22)]); line([(x1, m + 22), (x1, y2)])
        out.append(f'<rect x="{x1-8}" y="{m-22}" width="16" height="44" fill="white" stroke="{INK}" stroke-width="2"/>')
        text(x1 + 14, m - 2, ref, 12, weight="bold"); text(x1 + 14, m + 13, val, 12)
    else:
        m = (x1 + x2) / 2
        line([(x1, y1), (m - 22, y1)]); line([(m + 22, y1), (x2, y1)])
        out.append(f'<rect x="{m-22}" y="{y1-8}" width="44" height="16" fill="white" stroke="{INK}" stroke-width="2"/>')
        text(m, y1 - 14, f"{ref} {val}", 12, "middle", weight="bold")


def cap(x, y_top, ref, val):
    """Vertical capacitor from y_top down to GND."""
    line([(x, y_top), (x, y_top + 18)])
    line([(x - 14, y_top + 18), (x + 14, y_top + 18)], INK, 2.5)
    line([(x - 14, y_top + 25), (x + 14, y_top + 25)], INK, 2.5)
    line([(x, y_top + 25), (x, y_top + 40)])
    gnd(x, y_top + 40)
    text(x + 18, y_top + 16, ref, 12, weight="bold"); text(x + 18, y_top + 31, val, 12)


def ic(x, y, w, ref, part, left, right, pitch=34, title_extra=""):
    """IC body with pins. left/right: lists of (pin number, name) or None for a gap.
    Returns {name_or_number: (x, y)} of the outer pin ends."""
    n = max(len(left), len(right))
    h = n * pitch + 20
    box(x, y, w, h)
    text(x + w / 2, y - 26, ref, 15, "middle", weight="bold")
    text(x + w / 2, y - 9, part + title_extra, 13, "middle")
    ends = {}
    for side, pins in (("L", left), ("R", right)):
        for i, p in enumerate(pins):
            if p is None:
                continue
            num, name = p
            py = y + 20 + i * pitch
            if side == "L":
                line([(x - 34, py), (x, py)])
                text(x - 30, py - 5, str(num), 11, color=PIN)
                text(x + 8, py + 5, name, 12)
                ends[name] = ends[str(num)] = (x - 34, py)
            else:
                line([(x + w, py), (x + w + 34, py)])
                text(x + w + 30, py - 5, str(num), 11, "end", PIN)
                text(x + w - 8, py + 5, name, 12, "end")
                ends[name] = ends[str(num)] = (x + w + 34, py)
    return ends, h


def noconn(x, y):
    line([(x - 6, y - 6), (x + 6, y + 6)], "#999"); line([(x - 6, y + 6), (x + 6, y - 6)], "#999")


# ── frame ──────────────────────────────────────────────────────────────────
out.append(f'<rect x="0" y="0" width="{W}" height="{H}" fill="white"/>')
box(10, 10, W - 20, H - 20, fill="none")
text(30, 45, "Project Civic — dual CAN interface: STM32F407G-DISC1 (MB997) + 2 × NXP TJA1051T + 24LC512 + OBD-II", 22, weight="bold")
text(30, 70, "Net labels with the same name are connected. Every pin not drawn is left unconnected (open), exactly as on the board.", 13, color=NOTE)

# ── J1 OBD-II DLC (vehicle side, SAE J1962) ───────────────────────────────
obd_pins = [(i, n) for i, n in [(1, "—"), (2, "—"), (3, "—"), (4, "CHASSIS GND"), (5, "SIGNAL GND"), (6, "CAN-H (F-CAN)"),
                                  (7, "—"), (8, "—")]]
obd_r = [(i, n) for i, n in [(9, "—"), (10, "—"), (11, "—"), (12, "—"), (13, "—"), (14, "CAN-L (F-CAN)"), (15, "—"),
                               (16, "BATTERY +12 V")]]
j1, j1h = ic(80, 150, 230, "J1", "OBD-II DLC (SAE J1962), mating plug", [], obd_pins + obd_r, pitch=30)
for num, name in obd_pins + obd_r:
    xe, ye = j1[str(num)]
    if name == "—":
        noconn(xe + 4, ye)
text(80, 150 + j1h + 22, "— = not used, leave open", 12, color=NOTE)
x4, y4 = j1["4"]; x5, y5 = j1["5"]
line([(x4, y4), (x4 + 40, y4), (x4 + 40, y5)]); line([(x5, y5), (x4 + 40, y5)]); dot(x4 + 40, y5)
line([(x4 + 40, y5), (x4 + 40, y5 + 60)]); gnd(x4 + 40, y5 + 60)
x6, y6 = j1["6"]; line([(x6, y6), (x6 + 90, y6)]); netlabel(x6 + 90, y6, "FCAN_H")
x14, y14 = j1["14"]; line([(x14, y14), (x14 + 90, y14)]); netlabel(x14 + 90, y14, "FCAN_L")
x16, y16 = j1["16"]; line([(x16, y16), (x16 + 90, y16)]); netlabel(x16 + 90, y16, "VBAT_12V", color=PWR)

# ── Power: VBAT -> F1 -> D1 -> U3 5 V regulator (NXP TJA1051 Fig. 18) ─────
py = 165
PX = 640
netlabel(PX, py, "VBAT_12V", side="left", color=PWR)
line([(PX, py), (PX + 30, py)])
out.append(f'<rect x="{PX+30}" y="{py-9}" width="50" height="18" fill="white" stroke="{INK}" stroke-width="2"/>')
line([(PX + 36, py), (PX + 74, py)], INK, 1)
text(PX + 55, py - 16, "F1 1 A fuse", 12, "middle", weight="bold")
line([(PX + 80, py), (PX + 130, py)])
out.append(f'<polygon points="{PX+130},{py-11} {PX+130},{py+11} {PX+152},{py}" fill="{INK}"/>')
line([(PX + 153, py - 11), (PX + 153, py + 11)], INK, 2.5)
text(PX + 141, py - 16, "D1", 12, "middle", weight="bold"); text(PX + 141, py + 28, "reverse-polarity", 11, "middle", NOTE)
line([(PX + 153, py), (PX + 300, py)]); dot(PX + 220, py); cap(PX + 220, py, "C1", "*")
RX = PX + 300
box(RX, py - 30, 170, 80)
text(RX + 85, py - 10, "U3", 14, "middle", weight="bold")
text(RX + 85, py + 14, "5 V automotive regulator", 11, "middle")
text(RX + 85, py + 30, "≥ 500 mA, 12 V input", 11, "middle")
text(RX + 6, py - 14, "IN", 11); text(RX + 164, py - 14, "OUT", 11, "end")
line([(RX + 85, py + 50), (RX + 85, py + 66)]); gnd(RX + 85, py + 66)
line([(RX + 170, py), (RX + 390, py)]); dot(RX + 230, py); cap(RX + 230, py, "C2", "47 nF")
dot(RX + 310, py); cap(RX + 310, py, "C3", "10 µF")
rail(RX + 390, py, "+5V")
text(PX, py + 112, "* NXP Fig. 18: input capacitor size depends on the regulator; C2 47 nF per Fig. 18.", 11, color=NOTE)

# ── U1 / U2 TJA1051T ──────────────────────────────────────────────────────
def transceiver(x, y, ref, bus, tx, rx, s, canh, canl, rtx, rs, cdec):
    left = [(1, "TXD"), (2, "GND"), (3, "VCC"), (4, "RXD")]
    right = [(8, "S"), (7, "CANH"), (6, "CANL"), (5, "n.c.")]
    e, h = ic(x, y, 170, ref, "NXP TJA1051T (SO8)", left, right, pitch=40)
    # TXD: open-drain STM32 output + pull-up to +5 V
    tx_x, tx_y = e["TXD"]
    line([(tx_x, tx_y), (tx_x - 150, tx_y)]); dot(tx_x - 60, tx_y)
    netlabel(tx_x - 150, tx_y, tx, side="left")
    resistor(tx_x - 60, tx_y - 110, tx_x - 60, tx_y, rtx, "2.2 kΩ")
    rail(tx_x - 60, tx_y - 110, "+5V")
    # RXD: 5 V push-pull output -> FT pin
    rx_x, rx_y = e["RXD"]
    line([(rx_x, rx_y), (rx_x - 150, rx_y)]); netlabel(rx_x - 150, rx_y, rx, side="left")
    # GND / VCC
    g_x, g_y = e["GND"]; line([(g_x, g_y), (g_x - 20, g_y), (g_x - 20, g_y + 4)]); gnd(g_x - 20, g_y + 4)
    v_x, v_y = e["VCC"]; line([(v_x, v_y), (v_x - 100, v_y)])
    dot(v_x - 100, v_y); rail(v_x - 100, v_y, "+5V")
    cap(v_x - 100, v_y, cdec, "100 nF")
    text(v_x - 150, v_y + 75, "(close to pin 3)", 10, color=NOTE)
    # S: open-drain STM32 output + pull-up: HIGH = Silent by default (MCU reset/boot)
    s_x, s_y = e["S"]
    line([(s_x, s_y), (s_x + 180, s_y)]); dot(s_x + 90, s_y)
    netlabel(s_x + 180, s_y, s)
    resistor(s_x + 90, s_y - 100, s_x + 90, s_y, rs, "2.2 kΩ")
    rail(s_x + 90, s_y - 100, "+5V")
    h_x, h_y = e["CANH"]; line([(h_x, h_y), (h_x + 180, h_y)]); netlabel(h_x + 180, h_y, canh)
    l_x, l_y = e["CANL"]; line([(l_x, l_y), (l_x + 180, l_y)]); netlabel(l_x + 180, l_y, canl)
    n_x, n_y = e["n.c."]; noconn(n_x + 4, n_y)
    text(x + 85, y + h + 22, bus, 13, "middle", NET, "bold")
    return e, h


u1, _ = transceiver(820, 420, "U1", "F-CAN 500 kbit/s (bxCAN1)", "CAN1_TX", "CAN1_RX", "TJA1_S", "FCAN_H", "FCAN_L", "R1", "R2", "C4")
u2, _ = transceiver(820, 820, "U2", "B-CAN 125 kbit/s (bxCAN2)", "CAN2_TX", "CAN2_RX", "TJA2_S", "BCAN_H", "BCAN_L", "R3", "R4", "C5")
text(560, 660, "No bus termination on U1/U2: the car's bus is already terminated; this is a stub node (NXP Fig. 18 note **).", 11, color=NOTE)

# ── J3 B-CAN tap ──────────────────────────────────────────────────────────
j3, j3h = ic(1500, 1080, 150, "J3", "B-CAN tap (2-pin)", [(1, "CAN-H"), (2, "CAN-L")], [], pitch=34)
x, y = j3["CAN-H"]; line([(x, y), (x - 40, y)]); netlabel(x - 40, y, "BCAN_H", side="left")
x, y = j3["CAN-L"]; line([(x, y), (x - 40, y)]); netlabel(x - 40, y, "BCAN_L", side="left")
text(1500, 1080 + j3h + 20, "Body-bus tap point: NOT on the DLC per the available", 11, color=PWR)
text(1500, 1080 + j3h + 35, "Honda/LEMON pages — identify it before wiring U2's bus side.", 11, color=PWR)

# ── U4 24LC512 on I2C1 ────────────────────────────────────────────────────
u4, u4h = ic(1540, 470, 170, "U4", "Microchip 24LC512 (64 KB I²C EEPROM)",
             [(1, "A0"), (2, "A1"), (3, "A2"), (4, "VSS")], [(8, "VCC"), (7, "WP"), (6, "SCL"), (5, "SDA")], pitch=40)
ax = u4["A0"][0] - 30
for p in ("A0", "A1", "A2", "VSS"):
    px, pyy = u4[p]; line([(px, pyy), (ax, pyy)]); dot(ax, pyy)
line([(ax, u4["A0"][1]), (ax, u4["VSS"][1] + 20)]); gnd(ax, u4["VSS"][1] + 20)
text(ax - 4, u4["A0"][1] - 14, "address 0x50", 11, "end", NOTE)
vx, vy = u4["VCC"]; line([(vx, vy), (vx + 130, vy)]); dot(vx + 80, vy); netlabel(vx + 130, vy, "BOARD_3V", color=PWR)
cap(vx + 80, vy, "C6", "100 nF")
wx, wy = u4["WP"]; line([(wx, wy), (wx + 14, wy), (wx + 14, wy + 4)]); gnd(wx + 14, wy + 4)
sx, sy = u4["SCL"]; line([(sx, sy), (sx + 110, sy)]); netlabel(sx + 110, sy, "I2C1_SCL")
dx, dy = u4["SDA"]; line([(dx, dy), (dx + 110, dy)]); netlabel(dx + 110, dy, "I2C1_SDA")
text(1440, 470 + u4h + 60, "WP = VSS: writes enabled. Pull-ups: on the MB997 already (R29/R33 4.7 kΩ to VDD) — add none.", 11, color=NOTE)

# ── MB997 board connector view ────────────────────────────────────────────
mb_left = [
    ("P2-3", "5V  (input, UM1472 §7.4)", "+5V", PWR),
    ("P2-5", "3V  (output)", "BOARD_3V", PWR),
    ("P2-1", "GND", None, GNDC),
    ("P2-2", "GND", None, GNDC),
    ("P2-33", "PD1  CAN1_TX  (AF9)", "CAN1_TX", NET),
    ("P2-36", "PD0  CAN1_RX  (AF9)", "CAN1_RX", NET),
    ("P2-34", "PD2  TJA1 S", "TJA1_S", NET),
    ("P1-37", "PB13  CAN2_TX  (AF9)", "CAN2_TX", NET),
    ("P1-36", "PB12  CAN2_RX  (AF9)", "CAN2_RX", NET),
    ("P2-31", "PD3  TJA2 S", "TJA2_S", NET),
    ("P2-23", "PB6  I2C1_SCL  (AF4)", "I2C1_SCL", NET),
    ("P2-20", "PB9  I2C1_SDA  (AF4)", "I2C1_SDA", NET),
    ("P2-13", "PE4  START relay out", "START_RELAY", NET),
    ("P2-14", "PE5  ACC sense in", "ACC_SENSE", NET),
    ("P2-11", "PE6  IGN2 sense in", "IGN2_SENSE", NET),
]
mx, my, mw = 2180, 150, 400
pitch = 44
mh = len(mb_left) * pitch + 30
box(mx, my, mw, mh, fill="#eef6ff")
text(mx + mw / 2, my - 26, "MB997  STM32F407G-DISC1", 15, "middle", weight="bold")
text(mx + mw / 2, my - 9, "header pins per UM1472 Table 7", 12, "middle")
for i, (hp, desc, net, col) in enumerate(mb_left):
    yy = my + 25 + i * pitch
    line([(mx - 40, yy), (mx, yy)])
    text(mx - 36, yy - 6, hp, 11, color=PIN, weight="bold")
    text(mx + 10, yy + 5, desc, 12)
    if net:
        line([(mx - 40, yy), (mx - 60, yy)]); netlabel(mx - 60, yy, net, side="left", color=col)
    else:
        line([(mx - 40, yy), (mx - 110, yy), (mx - 110, yy + 6)]); gnd(mx - 110, yy + 6)
ny = my + mh + 30
for t, c in [("CN5 USB OTG FS (micro-B): to head-unit / laptop USB host —", NET),
             ("   CDC-ECM network adapter, dashboard http://192.168.7.1", NET),
             ("CN1 ST-LINK USB (mini-B): flashing / debug only", INK),
             ("PD4 = CS43L22 reset (on-board), firmware holds it LOW: do not wire", NOTE),
             ("All other P1/P2 pins: leave unconnected.", PWR)]:
    text(mx - 40, ny, t, 12, color=c, weight="bold" if c == PWR else "normal"); ny += 18

# ── J4 start / key sense (stages per hardware reference) ──────────────────
j4, j4h = ic(2180, 1080, 220, "J4", "to start relay / key-sense stages", [(1, "START_RELAY"), (2, "ACC_SENSE"), (3, "IGN2_SENSE")], [], pitch=34)
for n in ("START_RELAY", "ACC_SENSE", "IGN2_SENSE"):
    x, y = j4[n]; line([(x, y), (x - 30, y)]); netlabel(x - 30, y, n, side="left")
text(2180, 1080 + j4h + 20, "PE4: drives a 5 V-triggered relay that switches 12 V at the", 11, color=NOTE)
text(2180, 1080 + j4h + 35, "ignition START contact (not the starter load). PE5/PE6: only", 11, color=NOTE)
text(2180, 1080 + j4h + 50, "via protected 12 V input stages — not designed on this sheet.", 11, color=NOTE)

# ── notes ─────────────────────────────────────────────────────────────────
notes = [
    "Why open-drain TXD/S + 2.2 kΩ to +5 V: the TJA1051T has no VIO pin, so VIO = VCC = 5 V and TXD/S need",
    "  VIH ≥ 0.7·VIO = 3.5 V (NXP Table 7). 3.3 V push-pull cannot reach that. PD1/PD2/PD3/PB13 are 5 V-tolerant (FT,",
    "  DS8626) and the firmware drives them open-drain. RXD is a 5 V output into FT pins PD0/PB12 (no resistor needed).",
    "S pull-up = transceiver Silent while the MCU is in reset or booting; firmware pulls S LOW only in bxCAN Normal mode.",
    "Power budget: each TJA1051 draws up to 70 mA dominant (NXP Table 7) — feed U1/U2 from U3, never from the board's",
    "  5V pin (≤ 100 mA as an output, UM1472 §7.4). On the bench without 12 V, feed the +5V net from a 5 V supply.",
    "Grounds: OBD pins 4 and 5, U3, U1, U2, U4 and MB997 GND are one ground net.",
]
ny = 1250
text(30, ny - 22, "Notes", 15, weight="bold")
for t in notes:
    text(30, ny, t, 12); ny += 18
refs = "References: UM1472 rev 9 · MB997 schematic E-01 · DS8626 · RM0090 · NXP TJA1051 data sheet · Microchip DS20001754 · SAE J1962 / ISO 15765-4"
text(30, H - 30, refs, 11, color=NOTE)
text(W - 30, H - 30, "Project Civic · rev 2026-11-16", 11, "end", NOTE)

svg = (f'<svg xmlns="http://www.w3.org/2000/svg" width="{W}" height="{H}" viewBox="0 0 {W} {H}" '
       f'font-family="DejaVu Sans, Arial, sans-serif">\n' + "\n".join(out) + "\n</svg>\n")
dst = Path(__file__).resolve().parents[1] / "docs" / "schematic_civic_can_stm32f407_tja1051.svg"
dst.write_text(svg)
print(dst)
