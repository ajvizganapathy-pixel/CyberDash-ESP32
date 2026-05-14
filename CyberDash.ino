#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include <esp_task_wdt.h>
#include <SPI.h>

#include "config.h"
#include "state.h"
#include "display.h"
#include "sensors.h"
#include "audio.h"
#include "storage.h"
#include "webserver.h"

SensorState  g_state;
portMUX_TYPE g_stateMux = portMUX_INITIALIZER_UNLOCKED;

// ---------- Tasks ----------
static void taskSensors(void*) {
  esp_task_wdt_add(NULL);
  for (;;) {
    sensorsRead();
    esp_task_wdt_reset();
    vTaskDelay(pdMS_TO_TICKS(SENSOR_PERIOD_MS));
  }
}

static void taskAudio(void*) {
  esp_task_wdt_add(NULL);
  for (;;) {
    audioSampleOnce();
    esp_task_wdt_reset();
    vTaskDelay(pdMS_TO_TICKS(50));   // ~20 Hz dB updates
  }
}

static void taskDisplay(void*) {
  esp_task_wdt_add(NULL);
  for (;;) {
    displayRender();
    esp_task_wdt_reset();
    vTaskDelay(pdMS_TO_TICKS(UI_PERIOD_MS));
  }
}

static void taskLog(void*) {
  esp_task_wdt_add(NULL);
  for (;;) {
    storageLog();
    esp_task_wdt_reset();
    vTaskDelay(pdMS_TO_TICKS(LOG_PERIOD_MS));
  }
}

static void taskWeb(void*) {
  esp_task_wdt_add(NULL);
  for (;;) {
    ArduinoOTA.handle();
    webBroadcastState();
    esp_task_wdt_reset();
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

static void taskAlerts(void*) {
  esp_task_wdt_add(NULL);
  bool tA = false, hA = false, dA = false;
  for (;;) {
    SensorState s = stateSnapshot();
    if (!isnan(s.temperatureC)) {
      if (s.temperatureC > TEMP_ALERT_C && !tA) {
        tA = true;
        audioBeep(2000, 120);
        audioBeep(1500, 120);
      } else if (s.temperatureC < TEMP_ALERT_C - 1) tA = false;
    }
    if (!isnan(s.humidity)) {
      if (s.humidity > HUMID_ALERT && !hA) { hA = true; audioBeep(1200, 200); }
      else if (s.humidity < HUMID_ALERT - 2) hA = false;
    }
    if (s.soundDb >= 0) {
      if (s.soundDb > DB_ALERT && !dA) { dA = true; audioBeep(2500, 80); }
      else if (s.soundDb < DB_ALERT - 5) dA = false;
    }
    esp_task_wdt_reset();
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

// ---------- Setup ----------
static void wifiConnect() {
  WiFi.mode(WIFI_STA);
  WiFi.setHostname(HOSTNAME);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  uint32_t t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < 10000) delay(200);
  bool ok = WiFi.status() == WL_CONNECTED;
  stateUpdate([ok](SensorState& s){ s.wifiOk = ok; });
  if (ok) {
    MDNS.begin(HOSTNAME);
    ArduinoOTA.setHostname(HOSTNAME);
    ArduinoOTA.setPassword(OTA_PASSWORD);
    ArduinoOTA.begin();
  }
}

void setup() {
  Serial.begin(115200);
  delay(50);
  Serial.println(F("\n[CyberDash] boot"));

  // Shared VSPI bus for TFT + SD
  SPI.begin(PIN_TFT_SCK, PIN_TFT_MISO, PIN_TFT_MOSI);

  displayInit();
  displayBootAnimation();

  sensorsInit();
  storageInit();
  audioInit();
  wifiConnect();
  webInit();
  audioStartupChime();

  esp_task_wdt_init(WDT_TIMEOUT_S, true);

  // core 0: I/O-bound tasks
  xTaskCreatePinnedToCore(taskSensors, "sensors", 4096, NULL, 2, NULL, 0);
  xTaskCreatePinnedToCore(taskAudio,   "audio",   4096, NULL, 3, NULL, 0);
  xTaskCreatePinnedToCore(taskLog,     "log",     6144, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(taskAlerts,  "alerts",  3072, NULL, 2, NULL, 0);

  // core 1: UI + web
  xTaskCreatePinnedToCore(taskDisplay, "display", 6144, NULL, 2, NULL, 1);
  xTaskCreatePinnedToCore(taskWeb,     "web",     6144, NULL, 1, NULL, 1);
}

void loop() {
  vTaskDelete(NULL);   // Arduino loop task not needed
}
