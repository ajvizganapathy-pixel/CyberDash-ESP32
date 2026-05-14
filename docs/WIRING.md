# CyberDash — Wiring Reference

ESP32 Dev Module (38-pin). All pins are GPIO numbers, not header positions.

## Power

| Rail   | Source                                         |
| ------ | ---------------------------------------------- |
| 3V3    | ESP32 3V3 pin → BME280, INMP441, SD VCC, TFT VCC/LED |
| GND    | common ground rail                             |
| 5V/VIN | optional speaker amp; do **not** feed BME280 or INMP441 |

> The SD card module's onboard level shifter usually wants 5V on `VCC`; check your module. The CS/SCK/MOSI/MISO signals are 3.3 V logic from the ESP32.

## SPI bus (VSPI — shared between TFT and SD)

```
ESP32              ST7735 TFT             microSD module
─────              ──────────             ──────────────
GPIO18 ────── SCK ──────────────────────── SCK
GPIO23 ────── MOSI/SDA ─────────────────── MOSI
GPIO19 ────── MISO ─── (TFT has no MISO) ─ MISO
GPIO 5 ────── CS
GPIO 2 ────── DC / A0
GPIO 4 ────── RST
GPIO15 ────── BLK (PWM, LEDC ch1)
GPIO13 ───────────────────────────────── CS
```

Bus is initialised once in `CyberDash.ino`:

```cpp
SPI.begin(PIN_TFT_SCK, PIN_TFT_MISO, PIN_TFT_MOSI);
```

Both `Adafruit_ST7735` and `SD.begin(PIN_SD_CS, SPI, 20_000_000)` use this instance.

## I2C bus (BME280)

```
ESP32       BME280
─────       ──────
GPIO21 ──── SDA
GPIO22 ──── SCL
3V3    ──── VCC
GND    ──── GND
```

`sensorsInit()` probes both `0x76` and `0x77` (board-dependent).

## I2S bus (INMP441 MEMS mic)

```
ESP32       INMP441
─────       ───────
GPIO26 ──── SCK   (BCLK)
GPIO25 ──── WS    (LRCLK)
GPIO33 ──── SD    (DOUT)
GND    ──── L/R   (selects left channel, matches I2S_CHANNEL_FMT_ONLY_LEFT)
3V3    ──── VDD
GND    ──── GND
```

24-bit samples are left-justified in 32-bit slots; `audio.cpp` shifts down by 8.

## Speaker / piezo

```
ESP32       Piezo / amp
─────       ───────────
GPIO27 ──── signal (LEDC ch0 PWM tone)
GND    ──── GND
```

For a passive piezo: drive directly. For an 8 Ω speaker: add a small MOSFET driver or amp board — GPIO27 cannot source the current.

## Button

```
ESP32       Button
─────       ──────
GPIO 0 ──── one side
GND    ──── other side
```

GPIO0 is a boot-strapping pin. Internal pull-up + momentary-to-ground is safe at runtime, but never hold it low at reset (forces bootloader).

## Strapping pins in use

ESP32 strapping pins read at reset: **GPIO0, GPIO2, GPIO5, GPIO12, GPIO15**. CyberDash uses 0, 2, 5, 15. None should be externally pulled at reset — keep them on devices that are themselves high-impedance until released by firmware (TFT RST holds the panel quiet during boot).

## Power budget (rough)

| Device        | Idle    | Active peak |
| ------------- | ------- | ----------- |
| ESP32 (WiFi)  | 80 mA   | 240 mA      |
| ST7735 + LED  | 20 mA   | 50 mA       |
| BME280        | < 1 mA  | < 1 mA      |
| INMP441       | 1.4 mA  | 1.4 mA      |
| SD card       | < 1 mA  | 100 mA peak |
| **total**     | ~110 mA | **~400 mA** |

Plan for a 500 mA+ 3V3 supply or a buck regulator from 5V.
