#pragma once
#include <Arduino.h>

void displayInit();
void displayBootAnimation();
void displayRender();           // call from task_display
void displaySetBrightness(uint8_t pct);
