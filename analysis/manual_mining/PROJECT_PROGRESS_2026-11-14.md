Project Civic progress — 2026-11-14
Owner-verified vehicle baseline

The owner confirms that every sensor/component identified so far is physically present on their 2013 Civic 9th Gen Pakistan-market RHD, R18Z1. The owner has personally verified these components. This is the working physical configuration.

Implemented in ESP32 dashboard source
Automatic engine ECU monitoring
ECU Mode 01 support discovery starts when the vehicle state is key position II (ON) or RUN and the F-CAN ARM window is active. ARM is still an explicit operator action.
The poller requests only supported Mode 01 PIDs from the standard OBD request path. Known PIDs are decoded to engineering units; other supported PIDs are shown as raw bytes.
Automatic OBD requests are paced to no more than five requests per second.
The same monitoring session reads engine ECU DTCs using OBD services 03, 07 and 0A plus UDS 19 02 every 30 seconds. These are read-only requests.
Polling is cancelled when F-CAN is disarmed, its ARM timer expires, or the vehicle leaves key position II. A disarm/key transition aborts the active UDS request and removes queued automatic requests.
The dashboard harness verifies ARM starts polling, the Mode 01 support/PID sequence works, DTC reads are scheduled, and DISARM pauses the monitor.
DTC event journal and snapshots
ECU DTC replies create/update persistent event records on ESP32 LittleFS. The UDS response is reassembled by ECU, bus and byte offset before DTC decoding. Reassembly is bounded to a 512-byte DTC response payload per bus to fit ESP32 internal RAM.
Each new event gets up to 60 seconds of 1 Hz pre-trigger decoded telemetry and 10 seconds of post-trigger telemetry. CSV includes event ID, DTC, ECU, bus, event timestamp, uptime, vehicle state, RPM, speed, source, pedal byte, bus frame rates and the PID summary.
Once the ten post-trigger samples are written, the dashboard exposes a downloadable per-event CSV link.
Events remain Current/Unseen until explicitly marked seen, then move to Historical and survive dashboard reconnect/reboot. Repeated 30-second reports update the latest record. A report gap greater than 120 seconds is treated as a possible recurrence and creates a fresh event. After reboot, a still-present DTC may be presented as current again.
The journal holds up to 40 events. It evicts only the oldest acknowledged event; if all 40 remain unseen, it preserves them and reports capacity reached. Event CSVs are limited to eight retained snapshots; an older acknowledged snapshot can be pruned to make room.
Verification
Dashboard browser harness: pass, including current-to-history lifecycle, event CSV link, ARM/key gate, Mode 01 requests, DTC polling and DISARM cancellation.
ESP32 PlatformIO firmware build: pass (recorded by make test-esp32); linker reports 111,716 bytes of RAM (34.1%) and 990,281 bytes of application flash (75.6% of the configured app partition). Final ESP32 binary is 996,864 bytes.
Source whitespace check: pass.
Earlier in this work session, STM32 host C/Python tests and both main/USB STM32 image builds passed.

## To implement as other implementations progress

PE6/IGN2 (and PE5/ACC where used) must be connected through the documented protected automotive input stage before key-state-based ARM/monitoring can work.
Automatic DTC polling currently targets the selected/default standard 11-bit engine ECU path.
Event snapshots are 1 Hz decoded telemetry. Currently automatic event triggers are ECU-reported DTCs.
Factory ECU freeze-frame/onboard snapshot data remains separate and must be preserved.
Honda-enhanced HDS Data List polling, validated Honda DIDs, anomaly rules, normal-operation baselines and ML inference are later phases.
Single-button Write display and transmission-monitor evidence
Dashboard now has one global ENTER WRITE MODE / EXIT WRITE MODE toggle. The duplicate Read/Write tabs in the Operating Mode section have been removed. A single click changes the entire dashboard palette and reveals the write workspace; clicking the same button restores the normal palette.
The live Read panel remains visible. Wi-Fi AP password remains CANmonitor26.
Double-tap/click on a decoded live Mode 01 numeric value in Write display mode opens the numeric editor and mobile input keyboard. Send to staging stores the proposed value locally and shows it as STAGED · NOT SENT.
The existing explicit STM32 transmit ARM window and UDS confirmation rules are preserved. This is a vehicle-bus safety gate.
Added further HDS automatic-transmission signal names to HONDA_HDS_DATA_LIST_ITEMS.md: Input Shaft (Mainshaft) Speed, Output Shaft (Countershaft) Speed, ATP_P/R/N/D/D3-S/2-1/FWD/RVS, A/T R Switch and ATF Temp Sensor voltage. These are HDS Data List names.
2026-11-14 — Honda 2013 gear DID acquisition added
External OBDb Honda Civic signalset / CarGeek mirror found 22 2612 at Engine ECU logical address DA0E → 0E, with two 8-bit gear candidates at response data bits 352–359 and 368–375. Model-year exceptions are consistent with 2012–2013 having responses recorded.
Normal-fixed 29-bit request/response IDs derived from Honda diagnostic address and standard tester address F1: 18DA0EF1 → 18DAF10E; request bytes 22 26 12.
ESP32 dashboard now polls this DID read-only at 250 ms while ignition is ON/RUN and F-CAN is explicitly armed. It validates response prefix/length and displays both raw variants.
Poll stops with the existing automatic monitor on disarm/key-off. Raw values remain visible as last-known values but status reports polling stopped.
Browser harness validates extended addressing, positive response reassembly/offset decoding, correct values for both bytes.
Verification after integration: node firmware/tests/dashboard_harness.js passed, event journal contract tests passed 6/6, hardware pin/safety tests passed 7/7, git diff --check passed, and make -C firmware test-esp32 PIO=/home/s752/.platformio/penv/bin/pio passed (recorded 100%).