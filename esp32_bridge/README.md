# ESP32-S NodeMCU — ST-CAN Wi-Fi Bridge

Transparent Wi-Fi ↔ STM32F407 bridge. No STM firmware changes required.

```
iPhone / Android / Laptop
   Wi-Fi  "ST-CAN"  192.168.4.x
         ↕
      ESP32-S NodeMCU
         ↕  TCP (direct to STM over shared USB network)
      STM32F407  192.168.7.1
         ↕  USB CDC-ECM
      (host PC if needed)
```

## How it works

The ESP32 creates a Wi-Fi AP and runs TCP proxies on ports 80, 81, 23 and 2323.
When a Wi-Fi client connects, the ESP opens a matching TCP connection to
`192.168.7.1` on the STM and forwards data in both directions. The STM
firmware is completely unchanged.

## Wiring

| ESP32 GPIO | STM32 pin | Signal    |
|------------|-----------|-----------|
| GPIO16 RX2 | PA2       | USART2_TX |
| GPIO17 TX2 | PA3       | USART2_RX |
| GND        | GND       | common    |

> **Note:** The UART wiring is optional for the TCP proxy to work.
> The TCP proxy connects directly from the ESP's Wi-Fi stack to the STM's
> USB-network IP (192.168.7.1).  UART is included for future raw-frame use.

## Network layout

| Device        | IP            | Interface            |
|---------------|---------------|----------------------|
| STM32F407     | 192.168.7.1   | USB CDC-ECM          |
| ESP32 (USB)   | 192.168.7.2   | USB CDC-ECM (DHCP)   |
| ESP32 (Wi-Fi) | 192.168.4.1   | Wi-Fi AP             |
| Wi-Fi client  | 192.168.4.x   | Wi-Fi                |

## Ports bridged

| Port | Service             |
|------|---------------------|
|  80  | HTTP dashboard      |
|  81  | WebSocket CAN stream|
|  23  | GVRET / SavvyCAN    |
| 2323 | Raw observer        |

## Build and flash

### PlatformIO (recommended)

```bash
cd esp32_bridge
pip install platformio
pio run -t upload --upload-port /dev/ttyUSB0
pio device monitor --port /dev/ttyUSB0 --baud 115200
```

### Arduino IDE

1. Board: **ESP32 Dev Module** (or NodeMCU-32S)
2. Install libraries: **AsyncTCP** and **ESPAsyncWebServer** (by me-no-dev)
3. Open `src/main.cpp`, paste into a new sketch, upload.

## Usage

1. Flash the ESP32.
2. Connect the STM board to the ESP32 via USB OTG cable (STM OTG → ESP32 USB).
3. Power both boards.
4. On phone/laptop: connect to Wi-Fi **ST-CAN**, password **Discovery1**.
5. Open `http://192.168.7.1/` in browser — the STM dashboard loads.
6. Or open `http://192.168.4.1/status` to see bridge health.
7. SavvyCAN: connect to `192.168.4.1`, port 23 (GVRET).

## Status page

`http://192.168.4.1/status` shows uptime, connected clients and byte counts.

## Troubleshooting

| Symptom | Cause | Fix |
|---------|-------|-----|
| Dashboard doesn't load | ESP not on USB network | Check USB OTG cable; ping 192.168.7.1 from laptop first |
| `502 STM unreachable` | STM powered off | Power the STM board |
| Wi-Fi visible but no IP | DHCP conflict | Reconnect; AP gives 192.168.4.x |
| SavvyCAN can't connect | Port 23 blocked | Disable firewall on laptop |
