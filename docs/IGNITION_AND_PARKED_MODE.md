# Project Civic — Ignition and parked-mode wiring

## Purpose

The STM32 no longer infers ignition position from CAN traffic. Vehicle CAN modules may remain active after key-off, and CAN silence cannot reliably distinguish key position I from position II. Physical ignition inputs are authoritative.

## Ignition inputs

| Vehicle signal | STM32F407 input | Logic |
|---|---|---|
| ACC / key position I | PE5 | Active-high 3.3 V |
| IGN2 / key position II | PE6 | Active-high 3.3 V |

PE5 and PE6 are exposed on the STM32F4-Discovery MB997 expansion headers. Each vehicle 12 V source must pass through its own automotive-protected, isolated/conditioned input stage. The MCU pins must only see a valid 3.3 V logic signal. Never connect ACC or IGN2 12 V directly to a GPIO.

## State behaviour now implemented

- **OFF / parked:** PE5=0 and PE6=0 continuously for 2 seconds. Both CAN scans stop, the bxCAN controllers are placed in SILM (TJA1051 S inputs stay grounded), bxCAN is asked to enter sleep, active diagnostics/gateway/periodic TX are stopped, and any active transmit window is revoked. ESP32 host communications remain available for firmware management.
- **ACC / position I:** PE5=1 and PE6=0. After 100 ms debounce, both CAN controllers start independent passive bitrate scans. Transmit ARM is not allowed.
- **ON / position II:** PE6=1. After 100 ms debounce, CAN monitoring remains active and ARM is permitted.
- **RUN:** ON plus a fresh RPM measurement above the configured threshold. The dashboard's Engine Start control is disabled while RUN is reported.
- Leaving position II immediately revokes ARM, cancels periodic TX and gateway operation, and aborts any active starter sequence. The ON-to-ACC transition is debounced for 200 ms.
- The Engine Start button is accepted only in ON, not ACC or OFF.

CAN traffic is intentionally not used as a fallback for ignition detection. If PE5/PE6 are not wired, the application remains in OFF and does not start CAN scanning. This is intentional and prevents a quiet or sleeping bus from being mistaken for key state.

## ESP32 ↔ STM32 communication and ROM bootloader

Application UART (target firmware mapping):
- STM32 PB10 USART3_TX (AF7) → ESP32 GPIO16 UART RX
- STM32 PB11 USART3_RX (AF7) ← ESP32 GPIO17 UART TX
- 2,000,000 baud, 8N1; common ground
- USART3_TX uses DMA1 Stream3 Channel4. RX remains USART3 RXNE/ORE interrupt driven.

STM32 ROM bootloader (same physical UART):
- ESP32 GPIO26 → STM32 BOOT0
- ESP32 GPIO25 → STM32 NRST (open-drain reset control)
- No ESP32-to-PB2/BOOT1 wire. MB997 SB19 must be fitted ON to hold BOOT1 low.
- STM32 PB10 USART3_TX → ESP32 GPIO16 UART RX
- STM32 PB11 USART3_RX ← ESP32 GPIO17 UART TX
- Bootloader UART uses 57,600 baud, 8E1; app UART returns to 2 Mbaud 8N1.
- The updater must receive ROM 0x7F ACK before erase; missing ACK is failure.

The ESP32 can reset the STM32 into its ROM bootloader while the vehicle key is OFF. Keep the ESP32 and STM32 powered during an update; do not interrupt power while flash erase/program is in progress.

## Power and CAN safety

- Vehicle battery +12 V must pass through a fuse and an automotive-rated reverse-polarity/transient-protected DC/DC supply before it reaches any development board or transceiver.
- TJA1051 pin 3 is VCC (+5 V); pin 2 is GND. Place a 100 nF ceramic bypass capacitor close to each IC between pins 3 and 2.
- TJA1051 pin 8 (S) is grounded by the supplied schematic, so the transceiver remains in Normal mode. Firmware uses bxCAN SILM for passive scanning and disarmed state; no MCU S-pin control is used.
- TJA1051 pin 5 depends on the exact orderable suffix: plain T = NC; T/3 = VIO (3.3 V); T/E = EN.
- Do not add a 120 Ω termination across the vehicle OBD CAN pair.
- Keep the PE4 starter-relay output physically disconnected during software/bench bring-up.

## Current limitation / next power phase

The current parked loop uses WFI and continues to service SysTick and the ESP32 UART. Measure standby current after the board is wired on a fused bench supply. A later STOP-mode implementation must preserve a reliable wake path from PE5/PE6.

## Verification

After the ignition-input hardware is built, bench-test each input with a current-limited 3.3 V logic source before connecting the vehicle wires:
1. PE5 only → ACC.
2. PE6 only → ON.
3. PE6 plus a valid RPM signal → RUN.
4. Both low → OFF after 2 seconds.
5. Confirm CAN scan starts only on ACC/ON and stops on OFF.
6. Confirm ARM and START are rejected in OFF/ACC.
