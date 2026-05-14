#pragma once
#include <Arduino.h>

bool audioInit();          // I2S RX + LEDC speaker setup
void audioSampleOnce();    // one block from I2S → RMS → dB into state
void audioBeep(uint16_t hz, uint16_t ms);
void audioStartupChime();
