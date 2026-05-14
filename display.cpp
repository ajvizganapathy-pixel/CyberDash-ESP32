#include "display.h"
#include "config.h"
#include "state.h"
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include <WiFi.h>

static Adafruit_ST7735 tft(PIN_TFT_CS, PIN_TFT_DC, PIN_TFT_RST);

// ---- cyberpunk palette (RGB565) ----
static constexpr uint16_t CB_BG      = 0x0841;
static constexpr uint16_t CB_GRID    = 0x18C3;
static constexpr uint16_t CB_CYAN    = 0x07FF;
static constexpr uint16_t CB_MAGENTA = 0xF81F;
static constexpr uint16_t CB_GREEN   = 0x07E0;
static constexpr uint16_t CB_YELLOW  = 0xFFE0;
static constexpr uint16_t CB_RED     = 0xF800;
static constexpr uint16_t CB_DIM     = 0x4208;

static float   histT[60];
static uint8_t histIdx = 0;
static int     prevBar = -1;

static void cardFrame(int16_t x, int16_t y, int16_t w, int16_t h, const char* label) {
  tft.drawRect(x, y, w, h, CB_DIM);
  tft.drawFastHLine(x + 2, y, 6, CB_CYAN);
  tft.drawFastHLine(x + w - 8, y, 6, CB_MAGENTA);
  tft.setTextColor(CB_DIM, CB_BG);
  tft.setTextSize(1);
  tft.setCursor(x + 3, y + 3);
  tft.print(label);
}

static void drawHeader(const SensorState& s) {
  tft.fillRect(0, 0, 128, 12, CB_BG);
  tft.drawFastHLine(0, 12, 128, CB_CYAN);
  tft.setTextColor(CB_CYAN, CB_BG);
  tft.setTextSize(1);
  tft.setCursor(2, 2);
  tft.print(F("CYBERDASH"));
  tft.setTextColor(s.wifiOk ? CB_GREEN : CB_RED, CB_BG);
  tft.setCursor(80, 2);
  tft.print(F("WiFi"));
  tft.setTextColor(s.sdOk ? CB_GREEN : CB_RED, CB_BG);
  tft.setCursor(108, 2);
  tft.print(F("SD"));
}

static void drawValue(int16_t x, int16_t y, const char* v, uint16_t color) {
  tft.fillRect(x + 2, y + 12, 56, 14, CB_BG);
  tft.setTextColor(color, CB_BG);
  tft.setTextSize(2);
  tft.setCursor(x + 4, y + 13);
  tft.print(v);
}

static void drawSoundBar(int16_t db) {
  const int16_t x = 2, y = 92, w = 124, h = 8;
  if (prevBar < 0) {
    tft.drawRect(x, y, w, h, CB_DIM);
    prevBar = 0;
  }
  int target = db < 0 ? 0 : constrain((int)map(db, 30, 100, 0, w - 2), 0, w - 2);
  if (target == prevBar) return;
  tft.fillRect(x + 1, y + 1, w - 2, h - 2, CB_BG);
  for (int i = 0; i < target; i++) {
    uint16_t c = (i < (w - 2) * 0.6) ? CB_GREEN
               : (i < (w - 2) * 0.85 ? CB_YELLOW : CB_RED);
    tft.drawFastVLine(x + 1 + i, y + 1, h - 2, c);
  }
  prevBar = target;
}

static void drawSparkline(const float* buf, uint8_t idx, uint8_t n,
                          int16_t x, int16_t y, int16_t w, int16_t h, uint16_t col) {
  tft.drawRect(x, y, w, h, CB_DIM);
  tft.fillRect(x + 1, y + 1, w - 2, h - 2, CB_BG);
  float mn = 1e9, mx = -1e9;
  for (uint8_t i = 0; i < n; i++)
    if (!isnan(buf[i])) { mn = min(mn, buf[i]); mx = max(mx, buf[i]); }
  if (mx <= mn) return;
  int16_t px = -1, py = -1;
  for (uint8_t i = 0; i < n; i++) {
    uint8_t j = (idx + i) % n;
    if (isnan(buf[j])) continue;
    int16_t nx = x + 1 + (i * (w - 2)) / n;
    int16_t ny = y + h - 2 - (int16_t)((buf[j] - mn) * (h - 3) / (mx - mn));
    if (px >= 0) tft.drawLine(px, py, nx, ny, col);
    px = nx; py = ny;
  }
}

void displayInit() {
  for (auto& v : histT) v = NAN;
  pinMode(PIN_TFT_BL, OUTPUT);
  ledcSetup(LEDC_CH_BL, 5000, LEDC_RES_BITS);
  ledcAttachPin(PIN_TFT_BL, LEDC_CH_BL);
  ledcWrite(LEDC_CH_BL, 200);

  tft.initR(INITR_BLACKTAB);
  tft.setRotation(0);
  tft.fillScreen(CB_BG);
}

void displaySetBrightness(uint8_t pct) {
  ledcWrite(LEDC_CH_BL, map(constrain(pct, 0, 100), 0, 100, 0, (1 << LEDC_RES_BITS) - 1));
}

void displayBootAnimation() {
  tft.fillScreen(CB_BG);
  for (int r = 0; r < 70; r += 4) {
    tft.drawCircle(64, 70, r, (r & 4) ? CB_CYAN : CB_MAGENTA);
    vTaskDelay(pdMS_TO_TICKS(18));
  }
  tft.setTextColor(CB_CYAN);
  tft.setTextSize(2);
  tft.setCursor(14, 52);
  tft.print(F("CYBER"));
  tft.setTextColor(CB_MAGENTA);
  tft.setCursor(14, 72);
  tft.print(F("DASH"));
  tft.setTextColor(CB_DIM);
  tft.setTextSize(1);
  tft.setCursor(28, 100);
  tft.print(F("booting..."));
  tft.drawRect(14, 120, 100, 8, CB_CYAN);
  for (int i = 0; i < 96; i++) {
    tft.drawFastVLine(16 + i, 122, 4, CB_GREEN);
    vTaskDelay(pdMS_TO_TICKS(12));
  }
  tft.fillScreen(CB_BG);
  cardFrame(2,  16, 60, 32, "TEMP");
  cardFrame(66, 16, 60, 32, "HUMID");
  cardFrame(2,  54, 60, 32, "PRES");
  cardFrame(66, 54, 60, 32, "dB");
}

void displayRender() {
  SensorState s = stateSnapshot();
  drawHeader(s);

  char buf[12];
  dtostrf(s.temperatureC, 4, 1, buf); strcat(buf, "C");
  drawValue(2, 16, buf, CB_GREEN);
  dtostrf(s.humidity, 4, 0, buf);     strcat(buf, "%");
  drawValue(66, 16, buf, CB_CYAN);
  dtostrf(s.pressureHpa, 4, 0, buf);
  drawValue(2, 54, buf, CB_YELLOW);
  if (s.soundDb < 0) strcpy(buf, "--");
  else                itoa(s.soundDb, buf, 10);
  drawValue(66, 54, buf, CB_MAGENTA);

  drawSoundBar(s.soundDb);

  // Append sparkline sample at 1 Hz
  static uint32_t lastHist = 0;
  if (millis() - lastHist > 1000) {
    lastHist = millis();
    histT[histIdx] = s.temperatureC;
    histIdx = (histIdx + 1) % 60;
  }
  drawSparkline(histT, histIdx, 60, 2, 106, 124, 22, CB_MAGENTA);

  tft.fillRect(0, 132, 128, 8, CB_BG);
  tft.setTextColor(CB_DIM, CB_BG);
  tft.setTextSize(1);
  tft.setCursor(2, 133);
  if (s.wifiOk) {
    tft.print(F("IP "));
    tft.print(WiFi.localIP());
  } else {
    tft.print(F("offline"));
  }
}
