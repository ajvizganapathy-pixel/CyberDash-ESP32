#include "webserver.h"
#include "config.h"
#include "state.h"
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <LittleFS.h>
#include <SD.h>

static AsyncWebServer server(80);
static AsyncWebSocket ws("/ws");

// ---- fallback page if LittleFS missing /index.html ----
static const char FALLBACK_HTML[] PROGMEM =
  "<!doctype html><meta charset=utf-8><title>CyberDash</title>"
  "<body style='background:#070713;color:#22e3ff;font-family:monospace;padding:24px'>"
  "<h1>CYBERDASH</h1><p>LittleFS image not uploaded yet. Run:</p>"
  "<pre>pio run -t uploadfs</pre>"
  "<p><a style='color:#ff37c7' href='/api/data'>/api/data</a> &middot; "
  "<a style='color:#ff37c7' href='/log.csv'>/log.csv</a></p></body>";

static String jsonState() {
  SensorState s = stateSnapshot();
  String j; j.reserve(96);
  j  = "{\"t\":";  j += String(s.temperatureC, 2);
  j += ",\"h\":";  j += String(s.humidity,     2);
  j += ",\"p\":";  j += String(s.pressureHpa,  2);
  j += ",\"db\":"; j += String(s.soundDb);
  j += "}";
  return j;
}

static String jsonSys() {
  SensorState s = stateSnapshot();
  String j; j.reserve(160);
  j  = "{\"wifi\":";    j += (s.wifiOk ? "true" : "false");
  j += ",\"sd\":";      j += (s.sdOk   ? "true" : "false");
  j += ",\"bme\":";     j += (s.bmeOk  ? "true" : "false");
  j += ",\"uptimeMs\":";j += String(millis());
  j += ",\"heap\":";    j += String(ESP.getFreeHeap());
  j += ",\"rssi\":";    j += String(WiFi.RSSI());
  j += ",\"ip\":\"";    j += WiFi.localIP().toString();    j += "\"";
  j += ",\"host\":\"";  j += HOSTNAME;                     j += "\"";
  j += "}";
  return j;
}

static void onWsEvent(AsyncWebSocket*, AsyncWebSocketClient* c, AwsEventType type,
                      void*, uint8_t*, size_t) {
  if (type == WS_EVT_CONNECT) c->text(jsonState());
}

bool webInit() {
  if (WiFi.status() != WL_CONNECTED) return false;

  const bool fsOk = LittleFS.begin(true);

  ws.onEvent(onWsEvent);
  server.addHandler(&ws);

  // Static assets from LittleFS (no cache so OTA-uploaded fs is reflected immediately)
  if (fsOk) {
    server.serveStatic("/", LittleFS, "/")
          .setDefaultFile("index.html")
          .setCacheControl("no-cache");
  } else {
    server.on("/", HTTP_GET, [](AsyncWebServerRequest* r){
      r->send_P(200, "text/html", FALLBACK_HTML);
    });
  }

  server.on("/api/data", HTTP_GET, [](AsyncWebServerRequest* r){
    r->send(200, "application/json", jsonState());
  });
  server.on("/api/sys", HTTP_GET, [](AsyncWebServerRequest* r){
    r->send(200, "application/json", jsonSys());
  });
  server.on("/log.csv", HTTP_GET, [](AsyncWebServerRequest* r){
    if (SD.exists("/log.csv")) r->send(SD, "/log.csv", "text/csv", true);
    else r->send(404, "text/plain", "no log");
  });
  server.on("/health", HTTP_GET, [](AsyncWebServerRequest* r){
    r->send(200, "text/plain", "ok");
  });

  server.onNotFound([](AsyncWebServerRequest* r){
    r->send(404, "text/plain", "404");
  });

  server.begin();
  return true;
}

void webBroadcastState() {
  if (ws.count() == 0) return;
  ws.cleanupClients();
  ws.textAll(jsonState());
}
