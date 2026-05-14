#include "sensors.h"
#include "state.h"
#include "config.h"
#include <Wire.h>
#include <Adafruit_BME280.h>

static Adafruit_BME280 bme;

bool sensorsInit() {
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL, 400000);
  bool ok = bme.begin(0x76, &Wire) || bme.begin(0x77, &Wire);
  if (ok) {
    bme.setSampling(Adafruit_BME280::MODE_NORMAL,
                    Adafruit_BME280::SAMPLING_X2,
                    Adafruit_BME280::SAMPLING_X2,
                    Adafruit_BME280::SAMPLING_X2,
                    Adafruit_BME280::FILTER_X4,
                    Adafruit_BME280::STANDBY_MS_500);
  }
  stateUpdate([ok](SensorState& s){ s.bmeOk = ok; });
  return ok;
}

void sensorsRead() {
  float t = bme.readTemperature();
  float h = bme.readHumidity();
  float p = bme.readPressure() / 100.0f;
  if (isnan(t) || isnan(h)) return;
  stateUpdate([=](SensorState& s){
    s.temperatureC = t;
    s.humidity     = h;
    s.pressureHpa  = p;
  });
}
