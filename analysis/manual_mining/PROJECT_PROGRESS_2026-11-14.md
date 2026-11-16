Project Civic progress — 2026-11-14
Owner-verified vehicle baseline

The owner confirms that every sensor/component identified so far is physically present on their 2013 Civic 9th Gen Pakistan-market RHD, R18Z1. The owner has personally verified these components. This is the working physical configuration.

Implemented features (automatic ECU monitoring, DTC event journal and snapshots): Firmware implementation: see [FIRMWARE_FEATURES.md](../../FIRMWARE_FEATURES.md) §6, §7.
Verification
Dashboard browser harness: pass, including current-to-history lifecycle, event CSV link, ARM/key gate, Mode 01 requests, DTC polling and DISARM cancellation.
ESP32 PlatformIO firmware build: pass (recorded by make test-esp32); linker reports 111,716 bytes of RAM (34.1%) and 990,281 bytes of application flash (75.6% of the configured app partition). Final ESP32 binary is 996,864 bytes.
Source whitespace check: pass.
Earlier in this work session, STM32 host C/Python tests and both main/USB STM32 image builds passed.

## To implement as other implementations progress

Firmware implementation: see [FIRMWARE_FEATURES.md](../../FIRMWARE_FEATURES.md) §3, §6, §7, §9.
Write-mode display and Honda gear DID 2612 acquisition: Firmware implementation: see [FIRMWARE_FEATURES.md](../../FIRMWARE_FEATURES.md) §5, §8.
Added further HDS automatic-transmission signal names to HONDA_HDS_DATA_LIST_ITEMS.md: Input Shaft (Mainshaft) Speed, Output Shaft (Countershaft) Speed, ATP_P/R/N/D/D3-S/2-1/FWD/RVS, A/T R Switch and ATF Temp Sensor voltage. These are HDS Data List names.
Verification after integration: node firmware/tests/dashboard_harness.js passed, event journal contract tests passed 6/6, hardware pin/safety tests passed 7/7, git diff --check passed, and make -C firmware test-esp32 PIO=/home/s752/.platformio/penv/bin/pio passed (recorded 100%).