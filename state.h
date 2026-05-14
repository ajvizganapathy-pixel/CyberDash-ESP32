#pragma once
#include <Arduino.h>
#include <functional>

struct SensorState {
  float    temperatureC = NAN;
  float    humidity     = NAN;
  float    pressureHpa  = NAN;
  int16_t  soundDb      = -1;
  bool     wifiOk       = false;
  bool     sdOk         = false;
  bool     bmeOk        = false;
  uint32_t updatedMs    = 0;
};

extern SensorState g_state;
extern portMUX_TYPE g_stateMux;

inline SensorState stateSnapshot() {
  SensorState s;
  portENTER_CRITICAL(&g_stateMux);
  s = g_state;
  portEXIT_CRITICAL(&g_stateMux);
  return s;
}

inline void stateUpdate(const std::function<void(SensorState&)>& fn) {
  portENTER_CRITICAL(&g_stateMux);
  fn(g_state);
  g_state.updatedMs = millis();
  portEXIT_CRITICAL(&g_stateMux);
}
