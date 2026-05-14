# CyberDash

ESP32 cyberpunk environmental dashboard — TFT UI, I2S sound-level monitoring,
BME280 climate sensing, SD logging, async web dashboard, OTA.

## Architecture

FreeRTOS task layout (pinned cores):

| Task     | Core | Period   | Purpose                           |
| -------- | ---- | -------- | --------------------------------- |
| sensors  | 0    | 1000 ms  | BME280 read → shared state        |
| audio    | 0    | 50 ms    | I2S RMS → dB SPL → shared state   |
| log      | 0    | 10 s     | Append CSV row to SD              |
| alerts   | 0    | 500 ms   | Threshold beeps                   |
| display  | 1    | 33 ms    | TFT render (~30 FPS)              |
| web      | 1    | 1 s      | WS broadcast + OTA handle         |

Shared state in `g_state` guarded by `portMUX_TYPE` (see `state.h`).
Every task is registered with the watchdog (`WDT_TIMEOUT_S = 8`).

## Hardware

| Pin (GPIO) | Function          |
| ---------- | ----------------- |
| 18 / 23 / 19 | TFT SCK / MOSI / MISO (VSPI, shared with SD) |
| 5          | TFT CS            |
| 2          | TFT DC            |
| 4          | TFT RST           |
| 15         | TFT backlight (LEDC ch 1) |
| 13         | SD CS             |
| 21 / 22    | I2C SDA / SCL (BME280) |
| 26 / 25 / 33 | I2S BCLK / WS / DIN (INMP441) |
| 27         | Speaker (LEDC ch 0) |
| 0          | Button (boot pin, do not pull low at reset) |

Boot-strap GPIOs (0, 2, 5, 15) are used — keep them floating/idle at reset.

## Build

```
pio run                              # build
pio run -t upload                    # serial flash
pio device monitor                   # 115200 baud
pio run -e esp32dev-ota -t upload    # OTA flash (after first serial flash)
```

mDNS hostname: `cyberdash.local` · OTA password: `changeme` (change in `config.h`).

## Web

- `http://cyberdash.local/`           dashboard
- `http://cyberdash.local/api/data`   JSON
- `http://cyberdash.local/log.csv`    download log
- `ws://cyberdash.local/ws`           live stream

## Config

WiFi credentials, GPIO map, sample rates, alert thresholds — all in `config.h`.
Runtime config persisted to `/config.json` on SD via `storageReadConfig` /
`storageWriteConfig`.

## Files

- `CyberDash.ino`  task setup + WiFi/OTA bring-up
- `display.*`      ST7735 TFT cyberpunk UI
- `sensors.*`      BME280
- `audio.*`        I2S RMS + LEDC tone speaker
- `storage.*`      SD CSV log with 1 MB rotation
- `webserver.*`    AsyncWebServer + WebSocket
- `state.h`        thread-safe shared state
- `config.h`       all compile-time settings
