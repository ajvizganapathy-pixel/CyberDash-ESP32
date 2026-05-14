#pragma once
#include <Arduino.h>

bool   storageInit();
void   storageLog();              // append one CSV row from state
String storageReadConfig();       // /config.json
bool   storageWriteConfig(const String& json);
