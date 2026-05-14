#include "storage.h"
#include "config.h"
#include "state.h"
#include <SD.h>
#include <SPI.h>

static const char*    LOG_PATH       = "/log.csv";
static const uint32_t LOG_MAX_BYTES  = 1024UL * 1024UL;   // 1 MB → rotate

static void rotateIfNeeded() {
  File f = SD.open(LOG_PATH, FILE_READ);
  if (!f) return;
  uint32_t sz = f.size();
  f.close();
  if (sz < LOG_MAX_BYTES) return;
  char rotated[32];
  snprintf(rotated, sizeof(rotated), "/log_%lu.csv", (unsigned long)millis());
  SD.rename(LOG_PATH, rotated);
}

bool storageInit() {
  pinMode(PIN_SD_CS, OUTPUT); digitalWrite(PIN_SD_CS, HIGH);
  bool ok = SD.begin(PIN_SD_CS, SPI, 20000000);
  if (ok && !SD.exists(LOG_PATH)) {
    File f = SD.open(LOG_PATH, FILE_WRITE);
    if (f) { f.println(F("epoch_ms,temp_c,humidity,pressure_hpa,db")); f.close(); }
  }
  stateUpdate([ok](SensorState& s){ s.sdOk = ok; });
  return ok;
}

void storageLog() {
  SensorState s = stateSnapshot();
  if (!s.sdOk) return;
  rotateIfNeeded();
  File f = SD.open(LOG_PATH, FILE_APPEND);
  if (!f) {
    stateUpdate([](SensorState& st){ st.sdOk = false; });
    return;
  }
  f.printf("%lu,%.2f,%.2f,%.2f,%d\n",
           (unsigned long)millis(), s.temperatureC, s.humidity, s.pressureHpa, s.soundDb);
  f.close();
}

String storageReadConfig() {
  if (!SD.exists("/config.json")) return "{}";
  File f = SD.open("/config.json", FILE_READ);
  if (!f) return "{}";
  String j; while (f.available()) j += (char)f.read();
  f.close();
  return j;
}

bool storageWriteConfig(const String& json) {
  File f = SD.open("/config.json", FILE_WRITE);
  if (!f) return false;
  f.print(json); f.close(); return true;
}
