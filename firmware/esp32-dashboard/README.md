# Civic CAN ESP32 dashboard

- Wi-Fi: connect to `Civic-CAN-Monitor`, password `CANmonitor26`.
- Open `http://192.168.4.1/`. There is no web login.
- UART to the STM32: ESP32 GPIO16 (RX) ← STM32 PA2 (USART2_TX), ESP32 GPIO17 (TX) → STM32 PA3
  (USART2_RX), 2 Mbaud 8N1, common GND. PA9/PA10 are not used: on the Discovery they are the
  USB VBUS/ID lines (UM1472 Rev 9 Table 7). The wiring diagram is produced only after a 0-bug audit.
- When the last dashboard client disconnects, the ESP32 sends DISARM.
- STM32 upload through the ESP32 (ROM bootloader, AN3155) uses the bootloader's USART3 on
  PB10/PB11 (DS8626 Section 3.13): ESP32 GPIO26 ← PB10 (P1-34), GPIO25 → PB11 (P1-35),
  GPIO4 → BOOT0 (P2-21), GPIO27 → PB2/BOOT1 (P1-24, open-drain; R34 pulls it HIGH on the
  MB997 and SB19 is not fitted), GPIO5 → NRST (P1-6, open-drain). The ESP32 moves its UART
  to GPIO25/26 only while flashing. Not yet tested on the boards; the on-board ST-LINK
  stays the primary flashing path.
- Build: `pio run -d esp32-dashboard`.
- Read mode is passive: the STM32 stays in bxCAN silent mode and never ACKs or transmits.
- Write mode (STM32 `make firmware` image, `build/project_civic.bin`; the receive-only
  `make firmware-rx` bench image answers `TX_NOT_BUILT`): commands are accepted only from a
  dashboard client (session token from `/token`, handed out without login). ARM opens a 1–600 s window in which the
  STM32 may transmit; DISARM, timeout, bitrate re-acquisition or bus-off end it.
- Diagnostics (ISO-TP / UDS / OBD-II, own STM32 code): read services (OBD 01/02/03/06/07/09/0A,
  UDS 19, 22, 3E, sessions 01/03) need ARM. Every other service (OBD 04 clear DTCs, UDS 14 clear,
  11 ECU reset, 31 routines, 27, 2E, ...) also needs an explicit confirmation in the browser,
  which the STM32 checks again (`CONFIRM` flag).
- Firmware update: ESP32 and STM32 images are uploaded as multipart forms (no login).
  The ESP32 image is verified by the OTA layer before reboot. The STM32 image is checked
  (vector table) before erase and read back after writing (ST AN3155 bootloader).
- Software build success does not confirm UART/CAN wiring or live vehicle data.
