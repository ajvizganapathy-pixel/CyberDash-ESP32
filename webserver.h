#pragma once

bool webInit();
void webBroadcastState();   // call ~1 Hz to push via WebSocket
