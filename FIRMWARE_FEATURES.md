# PROJECT CIVIC — FIRMWARE FEATURES & IMPLEMENTATION INSTRUCTIONS

Single source for every firmware feature and implementation instruction in this repository (done and still to do).
Other `.md` files keep only hardware facts, setup steps, diagnostic evidence and build/verification records, and link here.
Pins, wiring and flashing: [HARDWARE_REFERENCE_AND_FLASH_PROCEDURE.md](HARDWARE_REFERENCE_AND_FLASH_PROCEDURE.md).

Status keys: **[DONE]** = in the current STM32 source · **[TODO]** = not yet implemented, or must still be confirmed on hardware · **[LEGACY-ESP32]** = ESP32-only and no longer used, because the ESP32 was retired on 2026-11-15.

---

## 1. Platform decision

- **[DONE]** The ESP32 has been removed from the project. The STM32F407 (MB997) now does every job the ESP32 used to do (owner decision, 2026-11-15).
- **[DONE]** The Wi-Fi AP is replaced by USB networking on CN5 (OTG-FS, PA11/PA12). The board appears as a CDC-NCM adapter named "Civic CAN Monitor" (USB ID cafe:4020). It gives the host 192.168.7.2–.5 by DHCP and serves at `http://192.168.7.1/`, with the DNS name `civic.usb`.
- **[DONE]** Remote management after installation is done over the CN5 USB cable (dashboard and `/update-stm32`). Route that cable to the cabin.

## 2. CAN buses

- **[DONE]** F-CAN uses CAN1 (PB8/PB9), preferred rate 500 kbit/s. B-CAN uses CAN2 (PB5/PB6), preferred rate 125 kbit/s (corrected from 33.33 kbit/s using service-manual pages 1684/1690).
- **[DONE]** Each bus has its own receive queue and bus label. F-CAN must work fully on its own from engine start to engine stop. If B-CAN is not wired, nothing may depend on it, and it must not disturb F-CAN or any other circuit.
- **[DONE]** Capture-integrity banner: queue drops per bus, and IDs beyond the 192-ID table.
- **[TODO]** Confirm F-CAN 500 k and B-CAN 125 k from real frames on the vehicle.

## 3. Ignition / vehicle state (PE5 = ACC, PE6 = IGN2; physical inputs only)

- **[DONE]** Ignition position is never inferred from CAN traffic. If PE5/PE6 are not wired, the state stays OFF.
- **[DONE]** OFF: both inputs low for 2 s. ARM, UDS, ECU scan, gateway, periodic TX and the starter all stop. Listen-only capture continues.
- **[DONE]** ACC (position I): PE5=1, PE6=0, 100 ms debounce. ARM is not allowed.
- **[DONE]** ON (position II): PE6=1, 100 ms debounce. ARM is allowed.
- **[DONE]** RUN: ON plus fresh RPM ≥ 400 (`VEH_RUN_RPM`). The dashboard's Engine Start control is disabled while RUN is shown.
- **[DONE]** Leaving position II immediately revokes ARM and cancels periodic TX, the gateway and any starter sequence. The ON→ACC transition is debounced for 200 ms.
- **[TODO]** The parked loop currently uses WFI. A STOP-mode version must keep a reliable wake-up from PE5/PE6.

## 4. Remote engine start (PE4 → relay)

- **[DONE]** Starter state machine in `Core/Src/remote_start.c`. PE4 HIGH engages the starter; LOW means off.
- **[DONE]** The starter releases the moment the engine is confirmed running (RPM threshold), just as a key springs back from START to RUN. If the engine does not start, it releases as soon as the user-set timeout expires: default 2000 ms, range 500–10000 ms, minimum crank 300 ms.
- Requirements:
  - PE4 drives the existing 5 V-triggered relay, which switches 12 V. The relay output joins at exactly the point the ignition switch connects in its START position, so no vehicle wire is cut. The only difference from the key is who closes that contact.
- **[TODO]** On the vehicle, confirm release when the engine is running, using live RPM.

## 5. Dashboard and network services (STM32 over USB-NCM)

- **[DONE]** HTTP on port 80 serves `firmware/web/index.html` gzip-compressed.
- **[DONE]** WebSocket on port 81 uses the JSON protocol, with a 15 s / 3 s / 2 heartbeat.
- **[DONE]** Other services:
  - GVRET / SavvyCAN / RealDash on TCP 23
  - read-only observer on TCP 2323
  - `/log.csv` (`?historical=1`) and `POST /log/clear`
  - `/event.csv?id=`
- **[DONE]** A single ENTER WRITE MODE / EXIT WRITE MODE toggle switches the whole dashboard palette and shows the write workspace; the live Read panel stays visible. In write mode, double-tapping a decoded Mode 01 numeric value opens a numeric editor. "Send to staging" stores the value locally as STAGED · NOT SENT.
- **[DONE]** `POST /update-stm32` does the following:
  1. stages the image in flash sectors 6–7
  3. copies the image over the application from RAM
  4. resets the board
  5. reads the result back

  No ROM bootloader, BOOT0 or NRST wiring is needed. `POST /update` (ESP32 OTA) replies that no ESP32 exists.
- **[DONE]** Flash map:
  - sectors 0–5: application (256 KB, linker limit)
  - sectors 6–7: update staging
  - sectors 8–9: `log.csv` / `log.1.csv`, rotating at 128 KB
  - sectors 10–11: event journal

  Erases run from RAM and keep draining both bxCAN FIFOs. Known limit: 10,000 erase cycles per sector. With 1 Hz logging, each log sector is erased roughly every 30–90 min of uptime.

## 6. Diagnostics (ISO-TP / UDS / OBD-II, STM32 code)

- **[DONE]** Automatic engine-ECU monitoring works like this:
  1. Discovers the supported Mode 01 PIDs, then polls only those.
  2. Decodes known PIDs to engineering units and shows the rest as raw bytes.
  3. Sends at most 5 requests per second.
  4. Reads DTCs every 30 s with OBD 03/07/0A and UDS 19 02.
  5. DISARM, ARM expiry or a key change aborts the active UDS request and clears the queued automatic requests.
- **[DONE]** Vehicle Systems block with Engine, Transmission and Other tables. It combines OBD-II Mode 01, broadcast decoding and 239 Honda DIDs for model year 2013, from OBDb/Honda-Civic (CC BY-SA 4.0). 197 of those DIDs have no published layout and are shown raw. They are read with service 22.
- **[TODO]** Automatic DTC polling currently targets only the default standard 11-bit engine-ECU path. Coverage of other F-CAN/B-CAN modules must be added.

## 7. DTC event journal and snapshots

- **[DONE]** An ECU DTC reply creates or updates a persistent event. UDS replies are reassembled per ECU, bus and byte offset (bounded at 512 bytes per bus).
- **[DONE]** Each event gets 60 s of 1 Hz pre-trigger and 10 s of post-trigger decoded telemetry, with at most 8 retained snapshots. Its CSV contains:
  - event ID, DTC, ECU, bus
  - timestamp, uptime, vehicle state
  - RPM, speed, source, pedal byte
  - per-bus frame rates and the PID summary

  A per-event CSV link appears once the post-trigger samples are written.
- **[DONE]** Event lifecycle:
  1. A new event is Current/Unseen.
  2. When marked seen, it moves to Historical, which survives reconnect and reboot.
  3. A repeat report updates the latest record. A gap of more than 120 s counts as a recurrence and creates a new event.
  4. The journal holds 40 events and evicts only the oldest acknowledged one. If all events are unseen, it keeps them and reports that capacity is reached.
- **[TODO]** Event model from the manual-mining design:
  - Triggers: anomalies, stale/missing frames, bus-off/error spikes, implausible cross-signal relationships, brownout and engine-state transitions. Currently only ECU-reported DTCs trigger events.
  - Fields: boot ID, monotonic time, wall time only when trustworthy, trigger/rule version, severity, raw-frame excerpt and checksum.
  - Capture window: configurable per trigger; 9 s pre / ≥ 1 s post for stall-type events, matching Honda's stall snapshot. Profile the RAM/flash budget first.
  - Storage-full policy: keep metadata and high-severity/recent events.
  - Engine stop clears only the live-value display. ECU self-shutdown can keep the ECU awake for up to 1 h after LOCK, so "engine stopped" and "ECU asleep" must be separate dashboard states.

## 8. Passive decoding, gear and reverse engineering

- **[DONE]** Passive decoding of 36 Honda Nidec / Acura ILX 2016 F-CAN broadcast messages (opendbc, `firmware/dbc`, table generated by `tools/dbc_to_c.py`). They appear in the "Decoded F-CAN Broadcast Signals" panel.
- **[DONE]** Gear LED shift chase (`Core/Src/gear_led.c`):
  - Order: GREEN LD4 → ORANGE LD3 → RED LD5 → BLUE LD6 on an upshift, reversed on a downshift.
  - Brightness: 160 ms Fourier square-wave envelope per LED, `b(t) = 1/2 + (2/π)·Σ_{k odd ≤ 2H−1} sin(kωt)/k` with H = gear, using 62.5 Hz software PWM. The chase lasts 640 ms, then the health LEDs resume.
  - Commands: `GEAR,<0..15>` replies `GEAR,ACK,<prev>,<new>,UP|DOWN|SAME`. `GEAR,CLEAR` forgets the gear; key OFF also clears it.
- **[DONE]** Gear sources:
  - (a) Default: broadcast 0x1A3 TRANS_TARGET_GEAR (byte 1, bits 3..0). Two identical frames are required, key ON only. Toggled with `GEARBCAST,ON|OFF`.
  - (b) UDS DID 2612. Request `22 26 12` on 18DA0EF1, response `62 26 12` on 18DAF10E, polled every 250 ms. Variant 2 = bits 352–359, variant 3 = bits 368–375, chosen by a drop-down. Two identical replies are needed before `GEAR,<n>` is sent.
  - The dashboard checkbox "STM32 LED shift chase" disables the chase and sends `GEAR,CLEAR`.
- **[DONE]** DA1E substitute candidates: DIDs 2301, 2322 and 3086 on 18DA1EF1/18DAF11E, round-robin every 500 ms. Raw hex only; nothing is decoded or acted on.
- **[DONE]** "State Capture & Bit Finder" (the SavvyCAN Discrete-State method), with labelled CSV export for baseline / ST Edge AI training data.
- **[TODO]** Final goal: decode every CAN frame/ID on the connected buses into real values, for troubleshooting controllers and sensors. Bring in the reverse-engineering and two-way bus features of SavvyCAN, Candle and similar open-source projects. The HDS Data List items to decode include MAF, MAP, TP A/B, APP, ECT1/2, IAT, ST fuel trim, CYL1–4 misfire, VTEC pressure switch, shaft speeds, ATP switches and ATF temperature.

## 9. Fault detection and ML (later phases)

- **[TODO]** Planned order:
  1. Finish read-only PID discovery.
  2. Add the Honda HDS Data List.
  3. Add verified module coverage.
  4. Add deterministic checks: range, stale signal, rate of change, cross-signal plausibility, and CAN health (timeouts, bus-off, error counters, traffic-rate change).
  5. Collect normal baselines: cold start, warm idle, cruise, acceleration, deceleration, AC load and hot restart.
  6. Add ML inference.
- **[TODO]** ML on the F407 must use the Cortex-M4 runtime (ST Edge AI Core 4.1 or NanoEdge AI), not Neural-ART. Profile RAM, flash and inference time on target.

## 11. Legacy ESP32 behaviour (reference only)

- **[LEGACY-ESP32]** Wi-Fi AP `Civic-CAN-Monitor` on channel 6, max 4 clients, at 192.168.4.1. Same ports as §5.
- **[LEGACY-ESP32]** UART link: ESP32 UART2 GPIO16/17 ↔ STM32 USART3 PB10/PB11, 2 Mbaud 8N1. STM32 TX uses DMA1 Stream3 Ch4; RX is driven by RXNE/ORE interrupts.
- **[LEGACY-ESP32]** STM32 ROM-bootloader update (AN3155) over the same wires, at 57,600 8E1. GPIO26 drives BOOT0 and GPIO25 drives NRST (open-drain). SB19 must be ON to hold BOOT1 low. Local `/update` OTA was verified before reboot.
- **[LEGACY-ESP32]** DTC journal on LittleFS (512 KB log rotation).
