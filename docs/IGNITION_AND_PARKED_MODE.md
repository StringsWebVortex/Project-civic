# Project Civic — Ignition and parked-mode wiring

## Purpose

Physical ignition inputs are authoritative. State behaviour (OFF/ACC/ON/RUN, debounce, ARM rules) and parked-mode plans: Firmware implementation: see [FIRMWARE_FEATURES.md](../FIRMWARE_FEATURES.md) §3.

## Ignition inputs

| Vehicle signal | STM32F407 input | Logic |
|---|---|---|
| ACC / key position I | PE5 | Active-high 3.3 V |
| IGN2 / key position II | PE6 | Active-high 3.3 V |

PE5 and PE6 are exposed on the STM32F4-Discovery MB997 expansion headers.

## Power and CAN safety

- TJA1051 pin 3 is VCC (+5 V); pin 2 is GND. Place a 100 nF ceramic bypass capacitor close to each IC between pins 3 and 2.
- TJA1051 pin 8 (S) is grounded by the supplied schematic, so the transceiver remains in Normal mode.
- TJA1051 pin 5 depends on the exact orderable suffix: plain T = NC; T/3 = VIO (3.3 V); T/E = EN.

## Verification

After the ignition-input hardware is built, bench-test each input with a current-limited 3.3 V logic source before connecting the vehicle wires:
1. PE5 only → ACC.
2. PE6 only → ON.
3. PE6 plus a valid RPM signal → RUN.
4. Both low → OFF after 2 seconds.
5. Confirm CAN scan starts only on ACC/ON and stops on OFF.
6. Confirm ARM and START are rejected in OFF/ACC.
