# PROJECT CIVIC — HARDWARE REFERENCE & FIRMWARE PROGRAMMING PROCEDURE

Project root: /home/s752/Project-civic  
Purpose: single-source hardware record for the target vehicle, controller boards, CAN transceivers, wiring, power, diagnostic interfaces, measured hardware identification, Reverse Engineer unidentified CAN incoming frames, Decode incoming CAN frames into actual data/values, unresolved electrical conditions, and exact programming/verification commands.

---

## 1. Target vehicle

| Item | Project-specific value | Evidence / qualification |
|---|---|---|
| Vehicle | Honda Civic, 9th generation, 2013 | Owner-identified vehicle |
| Market / Power steering | Pakistan market, right-hand drive | Owner confirmation |
| Engine | Honda R18Z1, 1.8 L | Owner confirmation / 2013 service-information set "/home/s752/Project-civic/LEMON 2013 Honda Civic LX, 4D Sedan, Automatic Trans, Trans Mfr CD BRZA" |
| Transmission | Automatic (P, R, N, D, D3, 2, 1) | Owner-identified vehicle configuration |
| Diagnostic connector | Standard 16-pin DLC | F-CAN access referenced at DLC pins 6 and 14 |
| Vehicle supply | Nominal 12 V automotive battery system | No project-specific fused DC/DC input module is recorded as installed |

The owner has confirmed that all sensors/components referenced in the project findings are physically present on this vehicle.

The selected service-information corpus is the 2013 non-hybrid Civic service-manual variant. It is suitable technical evidence for the R18Z1 Engine. "/home/s752/Project-civic/LEMON 2013 Honda Civic LX, 4D Sedan, Automatic Trans, Trans Mfr CD BRZA"

## 2. Vehicle communication networks

| Network | Vehicle access / rate | Project controller | Current status |
|---|---|---|---|
| F-CAN / powertrain CAN | DLC pin 6 = F-CAN-H; pin 14 = F-CAN-L; nominal 500 kbit/s | STM32 CAN1, PB8 RX / PB9 TX |
| B-CAN / body CAN | Separate network; nominal 125 kbit/s; not identified as a direct DLC pair | STM32 CAN2, PB5 RX / PB6 TX |

F-CAN and B-CAN are separate electrical networks.

The supplied isolated bench-harness drawing shows two 120-ohm end terminators per bus and a common-mode branch using 1.3 kOhm / 1.3 kOhm with 4.7 nF. This is a standalone bench arrangement. TJA1051 bare ic official NXP block diagram is also placed /home/s752/Project-civic/Screenshot From 2026-10-28 01-50-33.png, you can take this as reference if you needed.

## 3. STM32 controller board

### 3.1 Board and MCU identification (its a Board, not bare IC)

| Item | Value |
|---|---|
| Board family / project identification | ST STM32F407GDISC1, MB997E |
| MCU configured by firmware | STM32F407VGT6 |
| MCU package / class | LQFP100, Cortex-M4 with FPU |
| CPU clock configured in firmware | 168 MHz |
| External high-speed crystal configured | 8 MHz |
| Flash capacity measured by ST-LINK probe | 1,048,576 bytes (1 MiB) |
| SRAM reported by ST-LINK probe | 196,608 bytes (192 KiB) |
| Silicon ID reported by ST-LINK | 0x413; STM32F4x5/F4x7 family |
| Programming/debug interface | On-board ST-LINK/V2.1 |
| ST-LINK firmware reported | V2J37S26 |

The connected ST-LINK was detected and the target reported 1 MiB flash and 192 KiB SRAM. The source/linker target is STM32F407VGTX / STM32F407VGT6. ST lists the STM32F407VGT6 Discovery board with 1 MiB flash and 192 KiB RAM.

### 3.2 STM32 pin assignments used by the current source

| Function | STM32 pin | Alternate function / direction | Current project use |
|---|---|---|---|
| F-CAN CAN1 RX | PB8 | AF9, input | TJA1051 RXD output |
| F-CAN CAN1 TX | PB9 | AF9, open-drain output in source | TJA1051 TXD input |
| B-CAN CAN2 RX | PB5 | AF9, input | TJA1051 RXD output |
| B-CAN CAN2 TX | PB6 | AF9, open-drain output in source | TJA1051 TXD input |
| ESP32 UART receive path | PB10 / USART3_TX | AF7, TX | STM32 → ESP32 GPIO16 RX |
| ESP32 UART transmit path | PB11 / USART3_RX | AF7, RX | ESP32 GPIO17 TX → STM32 |
| ACC / key position I | PE5 | GPIO input, active-high 3.3 V | Vehicle/key state input |
| IGN2 / key position II | PE6 | GPIO input, active-high 3.3 V | Vehicle/key state input |
| Starter control output | PE4 | GPIO output; LOW = off, HIGH = active | Relay-control output; both engine start method should work, 1-key start, 2-esp dash board Engine start |
| On-board audio reset | PD4 | GPIO output, driven LOW | Holds CS43L22 codec in reset because PB6/PB9 are reused |
| Green user LED LD4 | PD12 | GPIO output | Main-loop/frame-processing activity |
| Orange user LED LD3 | PD13 | GPIO output | ESP32 UART telemetry queue activity |
| Red user LED LD5 | PD14 | GPIO output | MCU VDD low warning |
| Blue user LED LD6 | PD15 | GPIO output | CAN receive traffic activity |

The four MB997 user LEDs are four separate single-colour LEDs, not one four-colour RGB LED (LD1 is the ST-LINK COM LED and LD2 the 3 V power LED; neither is MCU-controllable). Their default firmware assignments are the system-health indicators above.

Gear-shift LED sequence: Firmware implementation: see [FIRMWARE_FEATURES.md](FIRMWARE_FEATURES.md) §8.

### 3.3 Current CAN controller settings

| Controller | Bus | RX / TX pins | AF | Preferred nominal bitrate |
|---|---|---|---|---:|
| CAN1 | F-CAN | PB8 / PB9 | AF9 | 500,000 bit/s |
| CAN2 | B-CAN | PB5 / PB6 | AF9 | 125,000 bit/s |

The values above are read from the current firmware/Core/Src/main.c controller configuration.

The TJA1051 S pins are designed to be grounded, so the transceiver itself remains in Normal mode. Listen-only/ARM behaviour and B-CAN independence: Firmware implementation: see [FIRMWARE_FEATURES.md](FIRMWARE_FEATURES.md) §2.

The physical markings of both transceiver bare ICs are shown in this file /home/s752/Pictures/Screenshots/Screenshot From 2026-10-28 01-46-44.png and its working modes shown in file /home/s752/Pictures/Screenshots/Screenshot From 2026-10-28 21-11-08.png

## 4. CAN transceiver ICs

The project interface specifies two NXP TJA1051 high-speed CAN transceivers: one for F-CAN and one for B-CAN.

## 5. ESP32 dashboard controller board

### 5.1 Board / chip identification

| Item | Value |
|---|---|
| Project board target | ESP-32S NodeMCU v1.1 / PlatformIO nodemcu-32s |
| Framework / PlatformIO environment | Arduino, esp32dev |
| Actual chip found by esptool | ESP32-D0WD-V3, revision v3.0 |
| CPU / core mode reported | Dual-core, 240 MHz |
| Crystal | 40 MHz |
| External flash measured | 4 MiB |
| USB serial bridge enumeration | QinHeng Electronics, USB Single Serial, VID:PID 1A86:55D4 |
| Current ESP32 serial port | /dev/ttyACM0 |
| Stable by-id symlink observed | /dev/serial/by-id/usb-1a86_USB_Single_Serial_5573010236-if00 |

The ST-LINK VCOM interface also enumerates as /dev/ttyACM1 with STMicroelectronics VID:PID 0483:374B. Do not use /dev/ttyACM1 as the ESP32 programming port. Direct ST-LINK/SWD programming uses the ST-LINK USB interface, not its VCOM serial port.

### 5.2 ESP32 GPIO / peripheral assignments

| Function | ESP32 pin | Direction / mode |
|---|---|---|
| STM32 USART3 receive | GPIO16 / UART2 RX | Receives STM32 PB10 USART3_TX |
| STM32 USART3 transmit | GPIO17 / UART2 TX | Sends to STM32 PB11 USART3_RX |
| STM32 BOOT0 control | GPIO26 | Push-pull; HIGH selects ROM boot, LOW selects normal application boot |
| STM32 NRST control | GPIO25 | Open-drain; LOW pulse resets, HIGH releases |
| USB debug console | USB-UART bridge | 115,200 bit/s console |
| Internal ESP32 flash filesystem | On-chip SPI flash / LittleFS | Dashboard persistent event journal |

ESP32 UART / ROM-boot behaviour (retired): Firmware implementation: see [FIRMWARE_FEATURES.md](FIRMWARE_FEATURES.md) §11.

The normal project flash script uses direct ST-LINK for the STM32; therefore it does not require the ESP32 GPIO25/GPIO26 ROM-boot control or SB19 strap.

### 5.3 ESP32 network interfaces configured by firmware

Legacy ESP32 network services (retired): Firmware implementation: see [FIRMWARE_FEATURES.md](FIRMWARE_FEATURES.md) §11.

## 6. Vehicle-side input, output and power conditioning

| Circuit | Project pin / interface | Required electrical condition | Current evidence |
|---|---|---|---|
| ACC input | STM32 PE5 | Separate automotive-protected / conditioned 12V to 5V input stage; active-high | Firmware input exists |
| IGN2 input | STM32 PE6 | Separate automotive-protected / conditioned 12V to 5V input stage; active-high | Firmware input exists |
| Starter relay command | STM32 PE4 | Logic output LOW = off, HIGH = active; drives the 5 V-triggered relay that switches 12 V at the ignition-switch START contact point, not a starter load directly | Behaviour: see FIRMWARE_FEATURES.md §4 |
| Board supply | Vehicle nominal 12 V | Fuse, reverse-polarity protection, automotive transient/load-dump protection, and regulated DC/DC rails |
| TJA1051 supply | Pin 3 | Regulated +5 V with local 100 nF decoupling capacitor installed |

## 7. Vehicle sensor and transmission evidence relevant to the hardware

The owner has confirmed the named engine (R18Z1) and transmission-related components are physically present. The 2013 R18Z1 manual's HDS Data List labels include:

- Engine/air path: MAF SENSOR, MAP SENSOR, TP SENSOR, TP SENSOR A/B, REL TP SENSOR, APP SENSOR, ENGINE SPEED, VEHICLE SPEED, IAT SENSOR, ECT SENSOR 1, ECT SENSOR 2.
- Fuel / combustion: AF FB (ST FUEL TRIM), CYL1 MISFIRE, CYL2 MISFIRE, CYL3 MISFIRE, CYL4 MISFIRE.
- VTEC: VTEC PRES SW.
- Automatic transmission: Input Shaft (Mainshaft) Speed, Output Shaft (Countershaft) Speed, ATP_P, ATP_R, ATP_N, ATP_D, ATP_D3/S, ATP_2-1, ATP_FWD, ATP_RVS, A/T R Switch / Reverse Switch (ATPRVS), ATF Temp Sensor (V).

These are HDS parameter names and physical diagnostic concepts. Decoding goal and reverse-engineering plan: Firmware implementation: see [FIRMWARE_FEATURES.md](FIRMWARE_FEATURES.md) §8.

### 7.1 Honda gear DID candidate added to the dashboard

Honda gear DID 2612, DA1E candidates and the LED shift chase: Firmware implementation: see [FIRMWARE_FEATURES.md](FIRMWARE_FEATURES.md) §8.

## 8. Current firmware image files

| Target | Image path | Size | SHA-256 |
|---|---|---:|---|
| STM32F407 default full firmware | firmware/build/project_civic.bin | 61,488 bytes | 8d344f0f58d29f49d7a158787835b70635231e68c97be8f76474bbffb1a86672 |
| STM32F407 optional USB-FS CDC image | firmware/build/usb/project_civic_usb.bin | 74,344 bytes | 86bfc57d639c26bc2c7e0d53c1c4fa5c358a63154a75958d069061555354b98c |
| ESP32 dashboard application image | firmware/esp32-dashboard/.pio/build/esp32dev/firmware.bin | 1,013,056 bytes | e0cdcb04e452f51cd9b705b3297941d4a45e38c4cab798a5351d0fbc7bbaf13a |

Built and flashed 2026-11-14 (flash_all run 20261114T182321Z, result 100%; log firmware/build/flash/flash_20261114T182321Z.log; pre-flash full-flash backups of both boards in backups/). Added in this build: STM32 `gear_led` module + `GEAR` host command (§3.2), ESP32 debounced gear forwarding and the DA1E 2301/2322/3086 read-only panel (§7.1), host tests `test_gear_led` / `test_gear_led_san`, and the dashboard harness checks for both. `image_has_full_code` now also requires the `GEAR,ACK` string in the STM32 image.

**Open hardware condition (unchanged, pre-existing):** on the bench USB supply the ESP32 still enters a brownout reset loop at `WiFi.mode(WIFI_AP)` (`BOOT_DIAG reset_reason=9`, "Brownout detector was triggered"), exactly as recorded in release/dual-bus-remap-20261113/BUILD-NOTES.md and firmware/build/flash/esp30.txt (2026-11-05). The ESP32 flash verifies byte-for-byte; the loop is a 3.3 V/5 V supply dip during RF calibration. Resolve per §10 step 7: power the NodeMCU from a short cable on a port/supply that holds ≥ 4.75 V at 500 mA (or feed 5 V on VIN from the regulated board rail), add bulk capacitance (≥ 470 µF on 5 V and 100 µF on 3.3 V), and confirm `WIFI_DIAG 5: softAP=1` on the 115,200-baud console.

### 8.1 Single-board build — ESP32 retired, STM32 performs all dashboard duties (2026-11-15)

Owner decision: the ESP32 is removed; the STM32 alone performs every ESP32 duty over USB networking on CN5 (OTG-FS, PA11/PA12), dashboard at http://192.168.7.1/. Services, update path and flash map: Firmware implementation: see [FIRMWARE_FEATURES.md](FIRMWARE_FEATURES.md) §1, §5.

| Target | Image path | Size | SHA-256 |
|---|---|---:|---|
| STM32F407 single-board image (default) | firmware/build/project_civic.bin | 220,116 bytes | 3f2dfc85cd17f21a82c369bfdc8668769fc083f893f0541478405fe7c8fa9311 |

Flashed by ST-LINK 2026-11-15, read-back byte-identical; on-target check over SWD: uptime advancing without resets (38 s observed), log and journal sectors formatted (`CLOG`/`CEVT`), one log row per second. Host tests `test_dash_bridge`, `test_dash_store`, `test_web_server` (+ sanitizer builds) and image checks `image_fits_app_sectors`, `image_has_dashboard` pass. **On-board USB test (CN5 connected, 2026-11-15):** enumerates as cafe:4020 "Civic CAN Monitor", DHCP gives the host 192.168.7.2, ping 1.7 ms; GET / returns the page byte-identical to web/index.html; /token, /log.csv, 404/400 replies and the civic.usb name work; WebSocket handshake and messages (status/frames/events_reset/veh) received, commands reach the STM32 (ARM refused with TX,ERR,KEY_NOT_ON while the key is OFF); GVRET TCP 23 keepalive F1 09 DE AD and 2 buses; observer TCP 2323 streams rows. Dashboard STM32 update: a bad image is rejected before erase; a good image is applied, the board restarts and the application flash equals the uploaded file. A coalesced-segment upload bug found on hardware was fixed and covered by test_web_server. **On-board remote-start bench check (PE4 not connected; key position II simulated by enabling the PE6 internal pull-up over SWD):** key OFF -> START,BLOCKED,KEY_NOT_ON; key II -> veh state ON; ARM refused with TX,F,ERR,NO_BITRATE_LOCK (no CAN bus); with the ARM interlock disabled for the bench run only, dashboard START -> START,CRANK, PE4 HIGH for ~1.5 s (configured limit) then LOW with START,TIMEOUT. Defaults restored afterwards. Features added in the 2026-11-15 builds: Firmware implementation: see [FIRMWARE_FEATURES.md](FIRMWARE_FEATURES.md) §2, §6, §8. Still to do on hardware: release on engine-running RPM, CAN capture with live frames, and the other section 10 bench/vehicle steps.

ESP32 supporting image files generated by PlatformIO:
- firmware/esp32-dashboard/.pio/build/esp32dev/bootloader.bin
- firmware/esp32-dashboard/.pio/build/esp32dev/partitions.bin
- /home/s752/.platformio/packages/framework-arduinoespressif32/tools/partitions/boot_app0.bin

The normal two-board flashing script uses the default STM32 image and the ESP32 bootloader/partition/boot_app0/application set. The optional STM32 USB-FS CDC image is a separate build; it is not the default STM32 image. It uses MB997 CN5 OTG-FS pins PA9/PA11/PA12.

The recorded build results for current STM32 default image, optional STM32 USB image, and ESP32 dashboard image are all 100%.

## 9. Programming tools and currently detected ports

| Tool / device | Current detection |
|---|---|
| ST-LINK USB | STMicroelectronics VID:PID 0483:374B; ST-LINK/V2.1 |
| ST-LINK programming utility | st-flash v1.8.0 |
| ST-LINK probe utility | st-info --probe |
| ESP32 USB serial | QinHeng VID:PID 1A86:55D4; /dev/ttyACM0 |
| ESP32 esptool | esptool.py v4.5.1 |
| PlatformIO Core | 6.2.0 |
| PlatformIO ESP platform | espressif32 6.9.0 |
| STM32 compiler | arm-none-eabi-gcc 14.2.1 |

Current port mapping:
- /dev/ttyACM0 = ESP32 USB Single Serial, use this for esptool.
- /dev/ttyACM1 = ST-LINK virtual COM interface, not the ESP32 programming port.
- ST-LINK/SWD programming uses the ST-LINK USB interface. In-vehicle updates: Firmware implementation: see [FIRMWARE_FEATURES.md](FIRMWARE_FEATURES.md) §5.

## 10. Physical bring-up order

1. Inspect both TJA1051 top markings and confirm pin-5 variant for each. Confirm pin 2 ground, pin 3 regulated 5 V, pin 8 S grounded, local 100 nF decoupling, and pin 6/7 CANL/CANH wiring. (already verified)
2. Confirm F-CAN at 500 kbit/s and B-CAN at 125 kbit/s from actual frames. Confirm bus labels, error counters, and listen-only state.
3. Build and validate separate protected ACC and IGN2 12 V input stages before wiring PE5/PE6. Starter-relay behaviour: Firmware implementation: see [FIRMWARE_FEATURES.md](FIRMWARE_FEATURES.md) §4.
4. The ESP32 is retired (2026-11-15); the STM32 performs all its duties. Firmware implementation: see [FIRMWARE_FEATURES.md](FIRMWARE_FEATURES.md) §1.
1. Verify ESP32 ↔ STM32 UART link at 2 Mbaud 8N1; verify ROM loader at 57,600 baud 8E1 only if using ESP32-controlled STM32 bootloader.
2. Confirm F-CAN OBD Mode 01 and DTC requests on the bench before connecting to the vehicle. For Honda DID 2612, require a live positive response from the target R18Z1 5AT and compare both variant bytes before assigning gear labels or LED actions.
3.  Confirm actual RHD B-CAN connector/cavity and wire colours from vehicle-specific diagram and physical continuity checks.
    Service-manual evidence (LEMON 2013 Civic set, network troubleshooting test points):
    | Bus | Gauge control module (tach) 32P | Under-dash fuse/relay box | DLC |
    |---|---|---|---|
    | F-CAN-H / F-CAN-L | No. 19 WHT / No. 20 RED | — | No. 6 WHT / No. 14 RED |
    | B-CAN-H / B-CAN-L | No. 30 PNK / No. 31 BLU (some pages: H = No. 26) | K(12P) 2 PNK / 1 BLU; M(7P) 2 PNK / 1 BLU; Q(16P) 5 PNK / 6 BLU | not present |
    F-CAN nodes per the manual: ECM/PCM, gauge control module, VSA modulator, steering angle sensor, EPS, SRS, TPMS, audio-navigation (if fitted). The F-CAN pair measures 46–58 Ω at the PCM (already terminated twice).

## 12. Primary technical references

- ST STM32F4DISCOVERY / MB997 board product and board specifications: https://www.st.com/en/evaluation-tools/stm32f4discovery.html
- ST MB997 E-01 schematic: https://www.st.com/resource/en/schematic_pack/mb997-f407vgt6-e01_schematic.pdf
- ST UM1472 board user manual: https://www.st.com/resource/en/user_manual/um1472-stm32f4-discovery-stmicroelectronics.pdf
- ST STM32F407VGT6 product / device specifications: https://estore.st.com/en/stm32f407vgt6-cpn.html
- NXP TJA1051 product datasheet (pin variants and electrical thresholds): https://www.nxp.com/docs/en/data-sheet/TJA1051.pdf
- Espressif ESP32 series datasheet: https://documentation.espressif.com/esp32_datasheet_en.html
- Community Honda Civic transmission signal-set listing for DID 2612 candidate: https://cargeek.org/vehicles/honda/civic/pids?category=transmission
- Community 29-bit Honda diagnostic addressing guide: https://cargeek.org/can/guides/obd-ii-and-uds-on-can
- 2013 Honda project source set: analysis/manual_mining/NETWORK_TOPOLOGY_MATRIX.md, analysis/manual_mining/R18Z1_ENGINE_WIRING_TEXT.md, analysis/manual_mining/HONDA_HDS_DATA_LIST_ITEMS.md, analysis/manual_mining/OWNER_VEHICLE_CONFIRMATION.md.

---

End of hardware reference.
