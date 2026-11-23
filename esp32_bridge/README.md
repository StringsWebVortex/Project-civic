# ESP32 Wi-Fi bridge

The ESP32 is the Wi-Fi/dashboard transport for the STM32F407 CAN controller. It is not a second CAN decoder: the STM32 owns CAN acquisition, DBC decoding, dashboard state, and command handling.

## Runtime topology

| Service | Address | Purpose |
|---|---|---|
| Wi-Fi AP | `ST-CAN` / `Discovery1` | Vehicle-local wireless network |
| Dashboard | `http://192.168.4.1/` | `firmware/web/index.html`, served from SPIFFS as gzip |
| Dashboard WebSocket | `ws://192.168.4.1:81/` | JSON telemetry and dashboard commands |
| Token API | `http://192.168.4.1/token` | Returns the WebSocket URL and session token |
| Pipeline diagnostics | `http://192.168.4.1/debug` | UART, WebSocket, heap, logging and flash-state counters |
| Bridge status | `http://192.168.4.1/bridge/status` | Uptime, UART bytes, AP clients and SPIFFS state |
| CAN log | `http://192.168.4.1/log` | Download buffered SPIFFS log |

The AP address is `192.168.4.1/24`. The dashboard WebSocket is on port **81**, path `/`. There is no TCP GVRET proxy on ports 23 or 2323 in this firmware.

The current 4 MB flash layout is: bootloader `0x1000`, partition table `0x8000`, OTA boot data `0xE000`, application slot 0 `0x10000`, application slot 1 `0x1F0000`, SPIFFS `0x3D0000` (128 KB), and coredump `0x3F0000` (64 KB).

## STM32 ↔ ESP32 wiring

Normal runtime UART is **2,000,000 baud, 8 data bits, no parity, 1 stop bit (8N1)**.

| STM32F407 | ESP32 DevKit | Direction |
|---|---|---|
| PA2 / USART2_TX | GPIO16 / RX2 | STM32 → ESP32 |
| PA3 / USART2_RX | GPIO17 / TX2 | ESP32 → STM32 |
| GND | GND | Common reference |
| NRST | GPIO26 | STM32 reset control |
| BOOT0 | GPIO27 | STM32 bootloader selection |

STM32 USART2 TX uses DMA1 Stream6 / Channel4 and a bounded SRAM queue. USART2 RX uses the RXNE interrupt and a line buffer. Dashboard telemetry is newline-delimited JSON; raw CAN CSV remains local to the STM32. The ESP32 forwards JSON lines to connected WebSocket clients. Browser text commands are reassembled per WebSocket client and forwarded as one newline-delimited JSON command; the STM32 routes them through its normal dashboard command handler.

The STM32 reserves one virtual WebSocket peer for the ESP32 bridge. `WS,CONNECT` and `WS,DISCONNECT` markers track the first/last Wi-Fi dashboard client. A periodic CONNECT heartbeat restores the virtual peer after an STM32 reset.

## Diagnostics

Open `http://192.168.4.1/debug` while connected to the AP. Important counters include:

- `uart_rx_bytes`, `uart_rx_lines`, `uart_json_lines`
- `uart_ignored_lines`, `uart_line_overflows`
- `ws_clients`, `wifi_clients`
- `stm32_diag_seen`, `stm32_diag_age_s`
- `stm32_uart_tx_dropped`, `stm32_uart_tx_dma_errors` and DMA TE/DME/FE subtype/last-flag fields, `stm32_uart_rx_dropped`
- `gvret_client`, `gvret_frames_sent`, `gvret_frames_dropped`
- `dbc_messages` (number of generated standard F-CAN definitions)
- `log_dropped_lines`, `log_buffered_bytes`
- `heap`, `spiffs_ok`, `flashing_stm`

The STM32 emits a compact `pipeline_diag` JSON record once per second. The ESP32 consumes it even when no browser is connected, so `/debug` can report the STM32 UART counters independently of the dashboard WebSocket. The age field identifies stale or missing STM32 reports. The dashboard also displays UART TX queue drops, DMA errors, UART RX drops, and GVRET TCP send backpressure.

The dashboard frame table is intentionally bounded at 192 distinct (ID, bus, format) identities. Its `over` counter counts frame updates rejected after the table fills. GVRET TCP port 23 is a separate raw-frame stream for one connected client; `gvret_frames_dropped` counts frames rejected because its TCP send buffer/write could not accept them. Neither route is asserted lossless without checking its corresponding counters.

DBC decoding currently applies only to standard F-CAN IDs with generated definitions. B-CAN, extended frames, and undefined IDs remain available as raw frames but are not decoded into signals.

`/bridge/status` provides a smaller bridge-health response. `/log` downloads the recent SPIFFS log and `/log/clear` clears it with HTTP POST.

## Firmware update

The STM32 update endpoints are `/firmware/upload` and `/update-stm32`. The ESP32 controls NRST and BOOT0, enters the STM32 ROM bootloader, and transfers the image over USART2 using **115200 baud, 8 data bits, even parity, 1 stop bit (8E1)**. This bootloader rate is separate from the normal 2 Mbaud runtime link.

ESP32 build and filesystem image:

```sh
pio run -d esp32_bridge
pio run -d esp32_bridge -t buildfs
```

Upload the ESP32 application and SPIFFS image using the corresponding PlatformIO `upload` and `uploadfs` targets. The ESP32 `data/index.html.gz` file must be regenerated whenever the dashboard source changes:

```sh
gzip -n -9 -c firmware/web/index.html > esp32_bridge/data/index.html.gz
```

Verify the compressed payload against the source before building the filesystem image:

```sh
cmp firmware/web/index.html <(gzip -cd esp32_bridge/data/index.html.gz)
```
