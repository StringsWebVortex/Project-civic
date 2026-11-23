# Schematic remap firmware — build and programming record (2026-11-13)

## Images

- stm32/project_civic.bin: STM32F407VG main firmware (60,272 bytes)
- stm32-usb/project_civic_usb.bin: optional STM32 USB-FS CDC image (73,096 bytes; built, not programmed)
- esp32/esp32_dashboard.bin: ESP32 dashboard (951,728 bytes)

ELF and linker map files are included alongside both STM32 images.

## Programming

- STM32F407VG main image installed by ST-Link at 0x08000000. ST-Link write
  verification passed, and two separate post-write read-backs compare exactly
  with the release BIN (SHA-256 in SHA256SUMS.txt).
- ESP32-D0WD-V3 dashboard installed through USB serial. The bootloader,
  partition table, OTA metadata and application segments passed both write
  hashes and separate esptool verify_flash checks.
- Full pre-program backups are in backups/: 1 MiB STM32 flash and 4 MiB
  ESP32 flash.
- ESP32 serial log confirms the dashboard begins startup, then reports
  Brownout detector was triggered during Wi-Fi AP+STA initialization and
  enters a reset loop. Flash verification passed.
  Check the USB supply/cable and measure the ESP32 3.3 V rail under Wi-Fi
  startup load.

Both boards were independently USB-powered during programming. They were not
interconnected, and no external CAN transceiver wiring or vehicle harness was
connected.
