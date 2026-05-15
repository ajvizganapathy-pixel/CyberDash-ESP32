#pragma once
#include <Arduino.h>

// ---------- Credentials ----------
// Real values live in `secrets.h` (gitignored). Copy `secrets.example.h`
// to `secrets.h` and fill in your Wi-Fi + OTA credentials before building.
#if __has_include("secrets.h")
  #include "secrets.h"
#else
  #include "secrets.example.h"
  #warning "secrets.h not found — using placeholder values from secrets.example.h"
#endif

// ---------- Pins ----------
#define PIN_TFT_SCK         18
#define PIN_TFT_MOSI        23
#define PIN_TFT_MISO        19
#define PIN_TFT_CS           5
#define PIN_TFT_DC           2
#define PIN_TFT_RST          4
#define PIN_TFT_BL          15

#define PIN_SD_CS           13

#define PIN_I2C_SDA         21
#define PIN_I2C_SCL         22

#define PIN_I2S_BCLK        26
#define PIN_I2S_WS          25
#define PIN_I2S_DIN         33

#define PIN_SPK             27
#define PIN_BTN              0

// ---------- LEDC ----------
#define LEDC_CH_SPK          0
#define LEDC_CH_BL           1
#define LEDC_RES_BITS        8

// ---------- App ----------
#define SAMPLE_RATE_HZ   16000
#define I2S_BUFFER_LEN     512
#define LOG_PERIOD_MS    10000
#define SENSOR_PERIOD_MS  1000
#define UI_PERIOD_MS       33     // ~30 FPS
#define WDT_TIMEOUT_S        8

#define TEMP_ALERT_C       32.0f
#define HUMID_ALERT        80.0f
#define DB_ALERT             85
