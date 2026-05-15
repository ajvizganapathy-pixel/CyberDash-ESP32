<div align="center">

# ⟁ CYBERDASH ⟁

**A production-grade ESP32 cyberpunk telemetry platform**
TFT dashboard · I2S sound monitoring · BME280 climate sensing · SD logging · live web UI · OTA

![Platform](https://img.shields.io/badge/platform-ESP32-22e3ff?style=for-the-badge&logo=espressif&logoColor=black)
![Framework](https://img.shields.io/badge/framework-Arduino-00979d?style=for-the-badge&logo=arduino&logoColor=white)
![Build](https://img.shields.io/github/actions/workflow/status/ajvizganapathy-pixel/CyberDash-ESP32/build.yml?branch=main&style=for-the-badge&logo=platformio&logoColor=white)
![Release](https://img.shields.io/github/v/release/ajvizganapathy-pixel/CyberDash-ESP32?style=for-the-badge&color=00e5c8)
![Downloads](https://img.shields.io/github/downloads/ajvizganapathy-pixel/CyberDash-ESP32/total?style=for-the-badge&color=ff37c7)
![License](https://img.shields.io/badge/license-MIT-lightgrey?style=for-the-badge)

**[⚡ Flash from your browser](https://ajvizganapathy-pixel.github.io/CyberDash-ESP32/flasher.html)** ·
**[📦 Latest Release](https://github.com/ajvizganapathy-pixel/CyberDash-ESP32/releases/latest)** ·
**[🌐 Project Site](https://ajvizganapathy-pixel.github.io/CyberDash-ESP32/)**

</div>

---

## ✦ Overview

CyberDash turns an ESP32 into a self-hosted, real-time environmental dashboard with a cyberpunk aesthetic on both the on-device TFT and the web UI. Sensors, audio RMS, SD logging, and the web server all run on separate FreeRTOS tasks pinned across both cores, coordinated through a single mutex-protected state struct.

## ✦ Features

- **FreeRTOS architecture** — 6 pinned tasks (sensors, audio, log, alerts, display, web), watchdog on every task
- **Thread-safe state** — single `SensorState` guarded by `portMUX_TYPE` critical sections
- **Cyberpunk TFT UI** — ST7735 128×160, animated boot, neon palette, sparkline, partial redraws (~30 FPS)
- **Live web dashboard** — LittleFS-served, WebSocket streaming, glassmorphism + neon glow, HTTP fallback
- **I2S sound metering** — INMP441 → 24-bit RMS → estimated dB SPL → threshold alerts
- **SD CSV logging** — 1 MB auto-rotation, `/log.csv` download endpoint
- **OTA over WiFi** — `ArduinoOTA` + mDNS (`cyberdash.local`)
- **Non-blocking everywhere** — no `delay()` in tasks, all timing via `vTaskDelay`/`millis`

## ✦ Architecture

```
              ┌──────── core 0 (I/O) ────────┐   ┌─────── core 1 (UI) ───────┐
              │                              │   │                           │
   I2C ──▶ sensors  ─┐                       │   │  display ─▶ ST7735 TFT    │
                     │                       │   │                           │
   I2S ──▶ audio    ─┤   stateUpdate() ──▶  g_state  ◀── stateSnapshot()    │
                     │   (portMUX_TYPE)      │   │     │                     │
   SPI ──▶ log      ─┘                       │   │     └─▶ web ──▶ /ws /api  │
                                             │   │                           │
              └──── alerts (beeper) ─────────┘   └───────── OTA ─────────────┘
```

Cross-community bridges (per `graphify-out/`): `stateSnapshot()` (betweenness 0.44), `stateUpdate()` (0.32), `displayRender()` (0.30).

## ✦ Hardware

| GPIO       | Function                                          |
| ---------- | ------------------------------------------------- |
| 18 / 23 / 19 | TFT SCK / MOSI / MISO (VSPI, shared with SD)    |
| 5          | TFT CS                                            |
| 2          | TFT DC                                            |
| 4          | TFT RST                                           |
| 15         | TFT backlight (LEDC ch 1, PWM)                    |
| 13         | SD CS                                             |
| 21 / 22    | I2C SDA / SCL (BME280)                            |
| 26 / 25 / 33 | I2S BCLK / WS / DIN (INMP441)                   |
| 27         | Speaker / piezo (LEDC ch 0)                       |
| 0          | Button (boot strap — keep idle at reset)          |

⚠️  GPIO 0, 2, 5, 15 are strapping pins — keep them at their default reset state.

## ✦ Project Layout

```
CyberDash-ESP32/
├── platformio.ini         build envs (esp32dev, esp32dev-ota)
├── CyberDash.ino          task setup, WiFi/OTA, watchdog init
├── config.h               pins, periods, thresholds, creds
├── state.h                shared SensorState + mutex helpers
├── display.{h,cpp}        ST7735 cyberpunk UI
├── sensors.{h,cpp}        BME280 I2C
├── audio.{h,cpp}          I2S RX + LEDC speaker
├── storage.{h,cpp}        SD CSV log + rotation + config.json
├── webserver.{h,cpp}      AsyncWebServer + WebSocket + LittleFS
├── data/                  → flashed to LittleFS
│   ├── index.html
│   ├── style.css
│   └── app.js
├── .vscode/               build/upload/monitor tasks
└── graphify-out/          knowledge-graph snapshot of the codebase
```

## ✦ Install

### Option A — Web Flasher (zero setup)

Plug the ESP32 into a Chrome- or Edge-based browser and visit
**[flasher.html](https://ajvizganapathy-pixel.github.io/CyberDash-ESP32/flasher.html)**.
The installer fetches the latest [GitHub Release](https://github.com/ajvizganapathy-pixel/CyberDash-ESP32/releases/latest),
detects the chip, and writes `bootloader.bin` / `partitions.bin` / `firmware.bin`
to the standard offsets. No toolchain needed.

> ⚠️ Release binaries ship with **placeholder credentials**. After first flash, connect
> to the device's serial console (or edit `secrets.h` and rebuild locally) to set your
> real Wi-Fi credentials.

### Option B — esptool (manual)

```bash
esptool.py --chip esp32 --port /dev/ttyUSB0 --baud 921600 write_flash \
  0x1000  bootloader.bin \
  0x8000  partitions.bin \
  0x10000 firmware.bin
```

### Option C — Build from source (PlatformIO)

```bash
# First-time setup
cp secrets.example.h secrets.h
# → edit secrets.h with your Wi-Fi + OTA password

# 1) Firmware (first flash via USB)
pio run -t upload

# 2) Web assets to LittleFS
pio run -t uploadfs

# 3) Serial monitor
pio device monitor

# Subsequent updates over WiFi (OTA)
pio run -e esp32dev-ota -t upload
pio run -e esp32dev-ota -t uploadfs
```

mDNS hostname: **`cyberdash.local`** · default OTA password: `changeme` (override in `secrets.h`).

## ✦ Releases & CI

| Workflow                                                   | Trigger              | Output                                    |
| ---------------------------------------------------------- | -------------------- | ----------------------------------------- |
| [`build.yml`](.github/workflows/build.yml)                 | push / PR to `main`  | Validates build; artifacts for 14 days    |
| [`release.yml`](.github/workflows/release.yml)             | tag `v*.*.*`         | Builds + publishes GitHub Release w/ bins |
| [`pages.yml`](.github/workflows/pages.yml)                 | change to root HTML  | Deploys site to GitHub Pages              |

**Cutting a release:**

```bash
# Bump version in version.h, update CHANGELOG.md
git commit -am "release: v1.1.0"
git tag v1.1.0
git push origin main --tags
# → release.yml builds firmware and attaches it to a new GitHub Release
```

Every release ships `firmware.bin`, `bootloader.bin`, `partitions.bin`, plus a
`manifest.json` (sha256 + flash offsets) consumed by the web flasher.

## ✦ Endpoints

| Path          | Purpose                                 |
| ------------- | --------------------------------------- |
| `/`           | Cyberpunk dashboard (LittleFS)          |
| `/ws`         | WebSocket — pushes `{t,h,p,db}` on tick |
| `/api/data`   | JSON snapshot (sensors only)            |
| `/api/sys`    | JSON system info (wifi, sd, heap, rssi) |
| `/log.csv`    | Download rotating CSV log               |
| `/health`     | `ok` — for uptime probes                |
| `/update`     | ArduinoOTA endpoint (gated by password) |

## ✦ Configuration

Edit `config.h`:

```cpp
#define WIFI_SSID    "Airtel_anja_6760"
#define WIFI_PASS    "air67968"
#define HOSTNAME     "cyberdash"
#define OTA_PASSWORD "changeme"

#define TEMP_ALERT_C  32.0f
#define HUMID_ALERT   80.0f
#define DB_ALERT      85
```

Runtime values can be persisted to `/config.json` on SD via `storageReadConfig()` / `storageWriteConfig()`.

## ✦ Roadmap

- [ ] Replace WiFi creds with WiFiManager captive portal
- [ ] HTTPS + auth for the web dashboard
- [ ] FFT-based audio spectrum view on TFT + web
- [ ] PSRAM frame buffer for tear-free TFT updates
- [ ] InfluxDB / MQTT exporter task
- [ ] Battery + INA219 power telemetry

## ✦ License

MIT © Anjan Ganapathy K
